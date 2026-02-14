#pragma once
#include "core/physics/collision.h"
#include "gameObjects/meshObject.h"
#include "core/physics/physicsQueue.h"
#include "core/input/keyboardInput.h"
#include "gameObjects/cameraObject.h"

class Player : public RigidBody
{
public:
	Player() = default;
	~Player() = default;

	float speed = 12;
	bool showCursor = false;

	KeyboardInput keyBoardInput;
	std::weak_ptr<CameraObject> camera;

	void Tick() override;
	void Start() override;
	
	void UpdateCamera();

private:

};