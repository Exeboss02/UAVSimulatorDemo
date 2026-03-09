#pragma once

#include "gameObjects/enemies/enemy.h"

class Drone : public Enemy {
public:
	Drone();
	~Drone() = default;
	void Start() override;
	void Tick() override;

private:
	void SetMoveSpeedMode(MoveSpeedMode mode) override;
	bool ShootAtPlayer(const float deltaTime) override;
};