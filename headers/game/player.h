#pragma once
#include "core/physics/collision.h"
#include "gameObjects/meshObject.h"
#include "core/physics/physicsQueue.h"
#include "core/input/keyboardInput.h"
#include "gameObjects/cameraObject.h"
#include "core/input/inputManager.h"
#include "core/input/controllerInput.h"
#include "core/audio/audioManager.h"
#include "core/assetManager.h"
#include "core/tools.h"

class Player : public RigidBody
{
public:
	Player() = default;
	~Player() = default;

	std::vector<SoundClip*> soundClips;
	std::weak_ptr<SoundSourceObject> speaker;

	float speed = 12;
	float mouseSensitivity = 0.006f;
	float cameraFov = 80.0f;

	DirectX::XMVECTOR moveVector = {};

	void LoadFromJson(const nlohmann::json& data) override;
	void SaveToJson(nlohmann::json& data) override;

	KeyboardInput keyBoardInput;
	std::weak_ptr<CameraObject> camera;

	void PhysicsTick() override;
	void Tick() override;
	void Start() override;
	
	void UpdateCamera();

	bool isPlayingMusic = false;

	Timer musicTimer;
	Timer sfxTimer;

private:
	float input[2] = {};
	bool showCursor = false;

	void shootRay();
};