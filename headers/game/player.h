#pragma once
#include "core/physics/collision.h"
#include "gameObjects/meshObject.h"
#include "core/physics/physicsQueue.h"
#include "core/input/keyboardInput.h"
#include "gameObjects/cameraObject.h"
#include "core/input/inputManager.h"
#include "core/input/controllerInput.h"

class Player : public RigidBody
{
public:
	Player() = default;
	~Player() = default;

	float speed = 12;
	float mouseSensitivity = 0.006f;
	float cameraFov = 80.0f;

	void LoadFromJson(const nlohmann::json& data) override;
	void SaveToJson(nlohmann::json& data) override;

	KeyboardInput keyBoardInput;
	std::weak_ptr<CameraObject> camera;

	void PhysicsTick() override;
	void Tick() override;
	void Start() override;
	
	void UpdateCamera();

private:
	float input[2] = {};
	bool showCursor = false;

	void shootRay();
};