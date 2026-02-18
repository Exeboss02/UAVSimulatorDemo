#include "scene/sceneManager.h"
#include "UI/button.h"
#include "UI/canvasObject.h"
#include "UI/text.h"
#include "core/filepathHolder.h"
#include "gameObjects/pointLightObject.h"
#include "gameObjects/room.h"
#include "gameObjects/turret.h"

// Very good macro, please don't remove
#define NAMEOF(x) #x

SceneManager::SceneManager(Renderer* rend) : mainScene(nullptr), renderer(rend), objectFromString(), isPaused(false) {
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

	// UI widget types
	this->objectFromString.RegisterType<UI::CanvasObject>(NAMEOF(UI::CanvasObject));
	this->objectFromString.RegisterType<UI::Widget>(NAMEOF(UI::Widget));
	this->objectFromString.RegisterType<UI::Button>(NAMEOF(UI::Button));
	this->objectFromString.RegisterType<UI::Text>(NAMEOF(UI::Text));

	this->objectFromString.RegisterType<Player>(NAMEOF(Player)); // Game specific

	CreateNewScene(this->emptyScene);
	this->emptyScene->CreateGameObjectOfType<CameraObject>();
}

void SceneManager::SceneTick() {
	if (!this->mainScene.get()) {
		this->mainScene = this->emptyScene;
	}

	PhysicsQueue::GetInstance().Tick();

	this->mainScene->SceneTick(this->isPaused);
	this->mainScene->SceneLateTick(this->isPaused);
	PhysicsQueue::GetInstance().ResetPhysicsTickCounter();
}

void SceneManager::LoadScene(Scenes scene) {
	switch (scene) {
	case Scenes::EMPTY:
		break;
	case Scenes::MAIN_MENU:
		LoadSceneFromFile((FilepathHolder::GetAssetsDirectory() / "scenes" / "MainMenu.scene").string());
		break;
	case Scenes::GAME:
		Logger::Warn("There is no game scene.");
		break;
	case Scenes::END_CREDITS:
		Logger::Warn("There is no end credits scene.");
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
}

void SceneManager::DeleteScene(std::shared_ptr<Scene>& scene) {
	scene.reset();
	RenderQueue::ClearAllQueues();
	Logger::Log("Deleted scene: ", this->currentScenePath);
}

void SceneManager::LoadSceneFromFile(const std::string& filePath) {
	CreateNewScene(this->mainScene);

	this->mainScene->finishedLoading = false;

	std::ifstream file(filePath);
	nlohmann::json data = nlohmann::json::parse(file);
	file.close();
	this->currentScenePath = filePath;

	// Actual loading
	CreateObjectsFromJsonRecursively(data["gameObjects"], std::shared_ptr<GameObject>(nullptr));

	this->mainScene->finishedLoading = true;
	this->mainScene->CallStartOnAll();

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
		RenderQueue::ChangeSkybox("space.dds");
	}
	if (ImGui::MenuItem("Asteroid")) {
		RenderQueue::ChangeSkybox("asteroids.dds");
	}
	if (ImGui::MenuItem("Planet")) {
		RenderQueue::ChangeSkybox("mars.dds");
	}
}

void SceneManager::SaveSceneToCurrentFile() {
	if (this->currentScenePath.empty()) {
		Logger::Log("No current scene file path set. Use Save As to choose a file.");
		return;
	}
	SaveSceneToFile(this->currentScenePath);
}

void SceneManager::InitializeSoundBank(std::string pathToSoundFolder) {
	AssetManager::GetInstance().InitializeSoundBank(pathToSoundFolder);
}

void SceneManager::AddSoundClipStandardFolder(std::string filename, std::string id) {
	AssetManager::GetInstance().AddSoundClipStandardFolder(filename, id);
}

void SceneManager::AddSoundClip(std::string path, std::string id) {
	AssetManager::GetInstance().AddSoundClip(path, id);
}

std::string SceneManager::GetPathToSoundFolder() { return AssetManager::GetInstance().GetPathToSoundFolder(); }

SoundClip* SceneManager::GetSoundClip(std::string id) { return AssetManager::GetInstance().GetSoundClip(id); }

void SceneManager::AudioManagerTick() { this->audioManager.Tick(); }

void SceneManager::InitializeMusicTrackManager(std::string pathToMusicFolder) {
	this->audioManager.InitializeMusicTrackManager(pathToMusicFolder);
}

void SceneManager::AddMusicTrackStandardFolder(std::string filename, std::string id) {
	this->audioManager.AddMusicTrackStandardFolder(filename, id);
}

void SceneManager::AddMusicTrack(std::string path, std::string id) { this->audioManager.AddMusicTrack(path, id); }

void SceneManager::PlayMusicTrack(std::string id) { this->audioManager.Play(id); }

void SceneManager::StopMusicTrack(std::string id) { this->audioManager.Stop(id); }

void SceneManager::FadeInPlayMusicTrack(std::string id, float startGain, float seconds) {
	this->audioManager.FadeInPlay(id, startGain, seconds);
}

void SceneManager::FadeOutStopMusicTrack(std::string id, float seconds) { this->audioManager.FadeOutStop(id, seconds); }

void SceneManager::GetMusicTrackSourceState(std::string id, ALint& sourceState) {
	this->audioManager.GetMusicTrackSourceState(id, sourceState);
}

void SceneManager::SetMusicTrackGain(std::string id, float gain) { this->audioManager.SetGain(id, gain); }

MusicTrack* SceneManager::GetMusicTrack(std::string id) { return this->audioManager.GetMusicTrack(id); }
