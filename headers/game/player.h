#pragma once
#include "core/assetManager.h"
#include "core/audio/audioManager.h"
#include "core/input/controllerInput.h"
#include "core/input/inputManager.h"
#include "core/input/keyboardInput.h"
#include "core/physics/collision.h"
#include "core/physics/physicsQueue.h"
#include "core/tools.h"
#include "game/hud.h"
#include "game/resourceManager.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/gun.h"
#include "gameObjects/meshObject.h"
#include <memory>

class Player : public RigidBody {
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

	void DecrementHealth(uint8_t hp);
	void IncrementHealth(uint8_t hp);
	uint8_t GetHealth() const;

	bool isPlayingMusic = false;
	bool canShoot = false;

	Timer musicTimer;
	Timer sfxTimer;
	Timer shootCoolDown;

	ResourceManager resources;
	std::unique_ptr<HUD> hud;

private:
	float input[2] = {};
	bool showCursor = true;

	uint8_t health;

	float cameraRotation[3];

	void shootRay();
	void Interact();

	void checkForTriggerPress();

	void aim();

	std::weak_ptr<Gun> gun;
};