#pragma once

#include <vector>
#include <memory>

#include "gameObjects/gameObject3D.h"
#include "core/physics/boxCollider.h"
#include "utilities/aStar.h"
#include "utilities/time.h"

enum MoveSpeedMode { 
	NORMAL, 
	SLOWED
};

class Enemy : public GameObject3D{
public:
	Enemy();
	~Enemy() = default;
	void Start() override;
	void Tick() override;

	void SetMoveSpeedMode(MoveSpeedMode mode);

	void SetPath(const std::vector<std::shared_ptr<AStarVertex>>& newPath);
private:
	int health;
	float movementSpeed;
	DirectX::XMVECTOR direction;
	DirectX::XMVECTOR targetRotation;
	const float rotationSpeed = 5.0f;

	std::weak_ptr<BoxCollider> hitbox;

	std::vector<std::shared_ptr<AStarVertex>> path;
	size_t maxPathIndex;
	size_t currentPathIndex;
	bool hasFinishedPath;

	const float damage = 5.f;
	bool canShoot = true;
	const float shotCooldown = 2.f;
	float timeSinceLastShot;

	void MoveAlongPath();
	bool IsAtCurrentPathNode();

	void UpdateShootCooldown();

	void ShootAtCore();
	void ShootAtPlayer();
};