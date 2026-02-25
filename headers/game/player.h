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
#include "gameObjects/gun.h"
#include "game/resourceManager.h"

class Player : public RigidBody
{
public:
	Player();
	~Player();

	std::vector<SoundClip*> soundClips;
	std::weak_ptr<SoundSourceObject> speaker;

	float speed = 12;
	float mouseSensitivity = 0.05f;
	float stickSensitivity = 1100.0f;
	float cameraFov = 80.0f;

	DirectX::XMVECTOR moveVector = {};

	void LoadFromJson(const nlohmann::json& data) override;
	void SaveToJson(nlohmann::json& data) override;

	KeyboardInput keyBoardInput;
	std::shared_ptr<ControllerInput> controllerInput;
	std::weak_ptr<CameraObject> camera;

	void PhysicsTick() override;
	void Tick() override;
	void Start() override;
	
	void UpdateCamera();

	void SetCameraRotation(float r, float p, float y);
	void OnCollision(std::weak_ptr<GameObject3D> gameObject3D);

	bool isPlayingMusic = false;
	bool canShoot = false;

	Timer musicTimer;
	Timer sfxTimer;
	Timer shootCoolDown;

	ResourceManager resources;

private:
	float input[2] = {};
	bool showCursor = true;

	float cameraRotation[3];

	void shootRay();
	void Interact();

	void checkForTriggerPress();

	void aim();

	std::weak_ptr<Gun> gun;
};