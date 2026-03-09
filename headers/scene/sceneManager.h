#pragma once

#include "core/assetManager.h"
#include "core/audio/soundEngine.h"
#include "core/physics/collision.h"
#include "core/physics/rigidBody.h"
#include "core/physics/testPlayer.h"
#include "core/tools.h"
#include "game/player.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/debugCamera.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/enemies/testEnemy.h"
#include "rendering/renderer.h"
#include "scene/objectFromStringFactory.h"
#include "utilities/masterVolume.h"
#include <memory>
#include <scene/scene.h>
#include <utilities/logger.h>

#include "core/eventManager.h"
#include "game/events.h"

// Forward-declare UI::Button to avoid including UI headers here
namespace UI {
class Button;
}

#include <fstream>
#include <nlohmann/json.hpp>

class SceneManager {
public:
	enum Scenes { EMPTY, MAIN_MENU, GAME, END_CREDITS, DEMO };

	SceneManager(Renderer* rend); // The renderer reference is temporary
	~SceneManager() = default;

	// Active scene manager access for editor/runtime wiring (nullable)
	static SceneManager* GetActive();

	// Wire a single UI::Button's stored event IDs to this SceneManager's EventManager
	void WireButtonEvents(class UI::Button* button);

	void SceneTick();

	/// <summary>
	/// For now this serves as a place to build scenes
	/// </summary>
	void LoadScene(Scenes scene);
	// Schedule a scene file to be loaded at the end of the current scene tick
	void QueueLoadSceneFile(const std::string& filePath);

	void SaveSceneToCurrentFile();
	void CreateNewScene(std::shared_ptr<Scene>& scene);
	void DeleteScene(std::shared_ptr<Scene>& scene);
	void LoadSceneFromFile(const std::string& filePath);
	void CreateObjectsFromJsonRecursively(const nlohmann::json& data, std::weak_ptr<GameObject> parent);
	void SaveSceneToFile(const std::string& filePath);

	void SetMainCameraInScene(std::shared_ptr<Scene>& scene);

	void TogglePause(bool enable);
	void SkyboxMenu();

private:
	bool IsCreditsScrollFinished(const std::shared_ptr<Scene>& scene) const;

	std::shared_ptr<Scene> mainScene;
	std::shared_ptr<Scene> emptyScene;
	ObjectFromStringFactory objectFromString;

	std::unique_ptr<EventManager> eventManager;

	Renderer* renderer; // This is temporary
	std::string currentScenePath;

	std::vector<std::unique_ptr<Mesh>> tempMeshes; // This is also temporary

	bool isPaused;

	// Active instance pointer
	static SceneManager* activeInstance;
};