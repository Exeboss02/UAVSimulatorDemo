#pragma once
#include "core/physics/collision.h"
#include "gameObjects/meshObject.h"
#include "core/physics/physicsQueue.h"

class TestPlayer : public RigidBody
{
public:
	TestPlayer();
	~TestPlayer();

	void Tick() override;
	void Start() override;

private:

};