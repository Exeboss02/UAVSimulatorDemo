#pragma once

#include <vector>
#include <memory>

#include "gameObjects/gameObject3D.h"
#include "utilities/aStar.h"
#include "utilities/time.h"
#include "core/physics/boxCollider.h"

class TestEnemy : public GameObject3D {
public:
	TestEnemy() : GameObject3D(), movementSpeed(1.5f), currentPathIndex(0), path({}) {}
	~TestEnemy() = default;
	void Start() override;
	void Tick() override;

	void SetPath(const std::vector<std::shared_ptr<AStarVertex>>& newPath);

private:
	const float movementSpeed;

	size_t currentPathIndex;
	std::vector<std::shared_ptr<AStarVertex>> path;
	std::weak_ptr<BoxCollider> hitBox;
	int health = 100;

};
