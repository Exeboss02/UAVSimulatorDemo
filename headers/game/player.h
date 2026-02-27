#pragma once
#include "core/assetManager.h"
#include "core/audio/audioManager.h"
#include "core/input/controllerInput.h"
#include "core/input/inputManager.h"
#include "core/input/keyboardInput.h"
#include "core/physics/collision.h"
#include "core/physics/physicsQueue.h"
#include "core/tools.h"
#include "game/health.h"
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
	std::weak_ptr<SoundSourceObject> storySpeaker;

	bool jumpInput = false;
	bool isGrounded = false;
	bool isJumping = false;
	float jumpForce = 12;

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

	// Show or hide the OS cursor and enable UI interaction
	void SetShowCursor(bool visible);

	// Enable or disable player movement/input (used when UI menus are open)
	void SetInputEnabled(bool enabled);

	void DecrementHealth(int hp);
	void IncrementHealth(int hp);
	int GetHealth() const;

	bool isPlayingMusic = false;
	bool canShoot = false;

	Timer musicTimer;
	Timer sfxTimer;

	ResourceManager resources;
	std::unique_ptr<HUD> hud;

	enum Guns {
		pistol,
		rifle
	};
	/// <summary>
	/// add gun based on enum, add to switch case to add more
	/// </summary>
	/// <param name="gun"></param>
	void addGun(Guns gunType);

private:
	float input[2] = {};
	bool inputEnabled = true;
	bool showCursor = true;

	Health health;

	float cameraRotation[3];

	void Interact();

	int interactDistance = 5;

	void CheckForTriggerPress();

	void Aim();

	std::weak_ptr<Gun> gun;
};