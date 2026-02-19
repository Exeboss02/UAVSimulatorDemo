#pragma once

#include "core/assetManager.h"
#include "core/audio/soundEngine.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/debugCamera.h"
#include "gameObjects/meshObject.h"
#include "rendering/renderer.h"
#include "core/physics/rigidBody.h"
#include "core/physics/collision.h"
#include "scene/objectFromStringFactory.h"
#include "utilities/masterVolume.h"
#include <memory>
#include <scene/scene.h>
#include <utilities/logger.h>
#include "core/physics/rigidBody.h"
#include "core/physics/testPlayer.h"
#include "game/player.h"
#include "gameObjects/testEnemy.h"
#include "core/tools.h"

#include <fstream>
#include <nlohmann/json.hpp>

class SceneManager {
public:
	enum Scenes { EMPTY, MAIN_MENU, GAME, END_CREDITS, DEMO };

	SceneManager(Renderer* rend); // The renderer reference is temporary
	~SceneManager() = default;

	void SceneTick();

	/// <summary>
	/// For now this serves as a place to build scenes
	/// </summary>
	void LoadScene(Scenes scene);

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
	std::shared_ptr<Scene> mainScene;
	std::shared_ptr<Scene> emptyScene;
	ObjectFromStringFactory objectFromString;

	Renderer* renderer; // This is temporary
	std::string currentScenePath;

	std::vector<std::unique_ptr<Mesh>> tempMeshes; // This is also temporary

	bool isPaused;
};