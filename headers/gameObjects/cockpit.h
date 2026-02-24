#pragma once 
#include "gameObjects/gameObject3D.h"

class Cockpit : public GameObject3D {
public:
	void Start() override;
	void SetOnDeath(std::function<void()> func);

private:
	std::function<void()> onDeath = []() { Logger::Log("Core Died"); };
	float health = 100;
};