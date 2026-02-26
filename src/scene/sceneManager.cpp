#include "scene/sceneManager.h"
#include "UI/button.h"
#include "UI/canvasObject.h"
#include "UI/image.h"
#include "UI/text.h"
#include "core/eventManager.h"
#include "core/filepathHolder.h"
#include "game/crosshair.h"
#include "game/events.h"
#include "game/gameManager.h"
#include "gameObjects/enemy.h"
#include "gameObjects/pointLightObject.h"
#include "gameObjects/room.h"
#include "gameObjects/turret.h"

// std
#include <Windows.h>

// Very good macro, please don't remove
#define NAMEOF(x) #x

// Define static active instance
SceneManager* SceneManager::activeInstance = nullptr;

SceneManager::SceneManager(Renderer* rend)
	: mainScene(nullptr), renderer(rend), objectFromString(), isPaused(false),
	  eventManager(std::make_unique<EventManager>()) {
	// set active instance for editor/runtime wiring
	SceneManager::activeInstance = this;
	this->objectFromString.RegisterType<GameObject>(NAMEOF(GameObject));
	this->objectFromString.RegisterType<GameObject3D>(NAMEOF(GameObject3D));
	this->objectFromString.RegisterType<MeshObject>(NAMEOF(MeshObject));
	this->objectFromString.RegisterType<SpotlightObject>(NAMEOF(SpotlightObject));
	this->objectFromString.RegisterType<CameraObject>(NAMEOF(CameraObject));
	this->objectFromString.RegisterType<DebugCamera>(NAMEOF(DebugCamera));
	this->objectFromString.RegisterType<SpaceShip>(NAMEOF(SpaceShip));
	this->objectFromString.RegisterType<Room>(NAMEOF(Room));
	this->objectFromString.RegisterType<BoxCollider>(NAMEOF(BoxCollider));
	this->objectFromString.RegisterType<SphereCollider>(NAMEOF(SphereCollider));
	this->objectFromString.RegisterType<RigidBody>(NAMEOF(RigidBody));
	this->objectFromString.RegisterType<SoundSourceObject>(NAMEOF(SoundSourceObject));
	this->objectFromString.RegisterType<PointLightObject>(NAMEOF(PointLightObject));
	this->objectFromString.RegisterType<TestPlayer>(NAMEOF(TestPlayer));
	this->objectFromString.RegisterType<Turret>(NAMEOF(Turret));
	this->objectFromString.RegisterType<TestEnemy>(NAMEOF(TestEnemy));
	this->objectFromString.RegisterType<Enemy>(NAMEOF(Enemy));

	// UI widget types
	this->objectFromString.RegisterType<UI::CanvasObject>(NAMEOF(UI::CanvasObject));
	this->objectFromString.RegisterType<UI::Widget>(NAMEOF(UI::Widget));
	this->objectFromString.RegisterType<UI::Button>(NAMEOF(UI::Button));
	this->objectFromString.RegisterType<UI::Text>(NAMEOF(UI::Text));
	this->objectFromString.RegisterType<Crosshair>(NAMEOF(Crosshair));
	this->objectFromString.RegisterType<UI::Image>(NAMEOF(UI::Image));

	// Game specific
	this->objectFromString.RegisterType<Player>(NAMEOF(Player)); // Game specific
	this->objectFromString.RegisterType<GameManager>(NAMEOF(GameManager));

	CreateNewScene(this->emptyScene);
	this->emptyScene->CreateGameObjectOfType<CameraObject>();

	AudioManager::GetInstance().Tick(); // for early initialization
}

SceneManager* SceneManager::GetActive() { return SceneManager::activeInstance; }

void SceneManager::WireButtonEvents(UI::Button* button) {
	if (!button) return;
	int clickId = button->GetOnClickEventID();
	int pressId = button->GetOnPressedEventID();
	int releaseId = button->GetOnReleasedEventID();

	if (clickId != 0) {
		button->SetOnClick([this, clickId]() { this->eventManager->Trigger(clickId); });
	}
	if (pressId != 0) {
		button->SetOnPressed([this, pressId]() { this->eventManager->Trigger(pressId); });
	}
	if (releaseId != 0) {
		button->SetOnReleased([this, releaseId]() { this->eventManager->Trigger(releaseId); });
	}
}

void SceneManager::SceneTick() {
#ifdef TIMER_DEBUG
	ImGui::Begin("Scene tick");
	const auto start{std::chrono::steady_clock::now()};
#endif

	if (!this->mainScene.get()) {
		this->mainScene = this->emptyScene;
	}

	PhysicsQueue::GetInstance().Tick();
	AudioManager::GetInstance().Tick();

	this->mainScene->SceneTick(this->isPaused);
	this->mainScene->SceneLateTick(this->isPaused);
	PhysicsQueue::GetInstance().ResetPhysicsTickCounter();

#ifdef TIMER_DEBUG
	const auto end{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{end - start};
	ImGui::Text(("Scene tick time: " + std::to_string(elapsedSeconds.count())).c_str());
	ImGui::End();
#endif
}

void SceneManager::LoadScene(Scenes scene) {
	switch (scene) {
	case Scenes::EMPTY:
		break;
	case Scenes::MAIN_MENU:
		LoadSceneFromFile((FilepathHolder::GetAssetsDirectory() / "scenes" / "MainMenu.scene").string());
		break;
	case Scenes::GAME:
		LoadSceneFromFile((FilepathHolder::GetAssetsDirectory() / "scenes" / "SpaceShipScene.json").string());
		break;
	case Scenes::END_CREDITS:
		LoadSceneFromFile((FilepathHolder::GetAssetsDirectory() / "scenes" / "EndCredits.scene").string());
		break;
	case Scenes::DEMO:
		LoadSceneFromFile((FilepathHolder::GetAssetsDirectory() / "scenes" / "testresult.json").string());
		break;
	default:
		break;
	}
}

void SceneManager::CreateNewScene(std::shared_ptr<Scene>& scene) {
	if (scene.get()) {
		DeleteScene(scene);
	}

	scene = std::make_shared<Scene>();
	scene->loadSceneCallback = [&](std::string filepath) { this->LoadSceneFromFile(filepath); };
}

void SceneManager::DeleteScene(std::shared_ptr<Scene>& scene) {
	// Clear scene-level events to avoid stale callbacks
	if (this->eventManager) {
		this->eventManager->ClearAllCallbacks();
	}

	scene.reset();
	RenderQueue::ClearAllQueues();
	Logger::Log("Deleted scene: ", this->currentScenePath);
}

void SceneManager::LoadSceneFromFile(const std::string& filePath) {
	CreateNewScene(this->mainScene);
	this->mainScene->sceneName = filePath;

	this->mainScene->finishedLoading = false;

	std::ifstream file(filePath);
	nlohmann::json data = nlohmann::json::parse(file);
	file.close();
	this->currentScenePath = filePath;

	// Actual loading
	CreateObjectsFromJsonRecursively(data["gameObjects"], std::shared_ptr<GameObject>(nullptr));

	this->mainScene->finishedLoading = true;
	this->mainScene->CallStartOnAll();

	// register listener for Play
	this->eventManager->RegisterCallback(static_cast<int>(ButtonEvent::PLAY), [this]() {
		Logger::Log("PLAY event triggered: loading GAME scene");
		this->mainScene->QueueLoadScene(
			(FilepathHolder::GetAssetsDirectory() / "scenes" / "SpaceShipScene.json").string());
	});

	// register listener for Exit
	this->eventManager->RegisterCallback(static_cast<int>(ButtonEvent::EXIT), []() {
		Logger::Log("EXIT event triggered: posting quit message");
		PostQuitMessage(0);
	});

	// After all objects are started, wire any buttons that were created/modified in the editor
	for (const auto& go : this->mainScene->GetGameObjects()) {
		if (auto btn = dynamic_cast<UI::Button*>(go.get())) {
			this->WireButtonEvents(btn);
		}
	}

	SetMainCameraInScene(this->mainScene);
}

void SceneManager::CreateObjectsFromJsonRecursively(const nlohmann::json& data, std::weak_ptr<GameObject> parent) {
	for (const nlohmann::json& objectData : data) {
		// Logger::Log(objectData.dump());

		if (!objectData.contains("type")) {
			throw std::runtime_error("Failed to load scene: GameObject doesn't have a type.");
		}

		GameObject* gameObjectPointer = static_cast<GameObject*>(objectFromString.Construct(objectData.at("type")));
		auto obj = std::shared_ptr<GameObject>(gameObjectPointer);
		this->mainScene->RegisterGameObject(obj);

		if (!parent.expired()) {
			obj->SetParent(parent);
		}

		if (objectData.contains("children")) {
			CreateObjectsFromJsonRecursively(objectData["children"], obj);
		}

		obj->LoadFromJson(objectData);

		if (auto btn = dynamic_cast<UI::Button*>(obj.get())) {
			int clickEid = 0;
			int pressedEid = 0;
			int releasedEid = 0;
			if (objectData.contains("onClickEvent") && objectData["onClickEvent"].is_number())
				clickEid = objectData["onClickEvent"].get<int>();

			if (objectData.contains("onPressedEvent") && objectData["onPressedEvent"].is_number())
				pressedEid = objectData["onPressedEvent"].get<int>();

			if (objectData.contains("onReleasedEvent") && objectData["onReleasedEvent"].is_number())
				releasedEid = objectData["onReleasedEvent"].get<int>();

			// Fallback to values stored on the Button instance
			if (clickEid == 0) clickEid = btn->GetOnClickEventID();
			if (pressedEid == 0) pressedEid = btn->GetOnPressedEventID();
			if (releasedEid == 0) releasedEid = btn->GetOnReleasedEventID();
		}
	}
}

void SceneManager::SaveSceneToFile(const std::string& filePath) {
	nlohmann::json data;

	int iterator = 0;
	for (size_t i = 0; i < this->mainScene->gameObjects.size(); i++) {
		if (this->mainScene->gameObjects[i]->GetParent().expired()) {
			this->mainScene->gameObjects[i]->SaveToJson(data["gameObjects"][iterator++]);
		}
	}

	// Logger::Log(this->mainScene->GetNumberOfGameObjects());
	// this->mainScene->QueueDeleteGameObject(light2);
	// Logger::Log("Loaded scene");

	////////////////
	std::ofstream outFile(filePath);
	outFile << data;
	outFile.close();
	this->currentScenePath = filePath;
}

void SceneManager::SetMainCameraInScene(std::shared_ptr<Scene>& scene) {
	if (auto newMainCamera = this->mainScene->FindObjectOfType<CameraObject>(); !newMainCamera.expired()) {
		newMainCamera.lock()->SetMainCamera();
	} else {
		Logger::Error("Couldn't find a camera in the scene.");
	}
}

void SceneManager::TogglePause(bool enable) { this->isPaused = enable; }

void SceneManager::SkyboxMenu() {
	if (ImGui::MenuItem("Old town")) {
		RenderQueue::ChangeSkybox("cubeMap.dds");
	}
	if (ImGui::MenuItem("Space")) {
		RenderQueue::ChangeSkybox("bright_space.dds");
	}
	if (ImGui::MenuItem("Asteroid")) {
		RenderQueue::ChangeSkybox("bright_asteroid.dds");
	}
	if (ImGui::MenuItem("Planet")) {
		RenderQueue::ChangeSkybox("bright_planet.dds");
	}
}

void SceneManager::SaveSceneToCurrentFile() {
	if (this->currentScenePath.empty()) {
		Logger::Log("No current scene file path set. Use Save As to choose a file.");
		return;
	}
	SaveSceneToFile(this->currentScenePath);
}
