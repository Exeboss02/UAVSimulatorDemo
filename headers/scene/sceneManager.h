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

	// Sound effects
	void InitializeSoundBank(std::string pathToSoundFolder); // end the path with /
	void AddSoundClipStandardFolder(std::string filename, std::string id);
	void AddSoundClip(std::string path, std::string id);
	std::string GetPathToSoundFolder();
	SoundClip* GetSoundClip(std::string id);

	// Music
	void AudioManagerTick();
	void InitializeMusicTrackManager(std::string pathToMusicFolder); // always end path with /
	void AddMusicTrackStandardFolder(std::string filename, std::string id);
	void AddMusicTrack(std::string path, std::string id);
	void PlayMusicTrack(std::string id);
	void StopMusicTrack(std::string id);
	void FadeInPlayMusicTrack(std::string id, float startGain, float seconds);
	void FadeOutStopMusicTrack(std::string id, float seconds);
	void GetMusicTrackSourceState(std::string id, ALint& sourceState);
	void SetMusicTrackGain(std::string id, float gain);
	MusicTrack* GetMusicTrack(std::string id);

	void SaveSceneToCurrentFile();
	void CreateNewScene(std::shared_ptr<Scene>& scene);
	void DeleteScene(std::shared_ptr<Scene>& scene);
	void LoadSceneFromFile(const std::string& filePath);
	void CreateObjectsFromJsonRecursively(const nlohmann::json& data, std::weak_ptr<GameObject> parent);
	void SaveSceneToFile(const std::string& filePath);

	void SetMainCameraInScene(std::shared_ptr<Scene>& scene);

	void TogglePause(bool enable);

private:
	std::shared_ptr<Scene> mainScene;
	std::shared_ptr<Scene> emptyScene;
	ObjectFromStringFactory objectFromString;
	AudioManager audioManager;

	Renderer* renderer; // This is temporary
	std::string currentScenePath;

	std::vector<std::unique_ptr<Mesh>> tempMeshes; // This is also temporary

	bool isPaused;
};