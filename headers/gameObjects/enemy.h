#pragma once

#include <vector>
#include <memory>

#include "gameObjects/gameObject3D.h"
#include "core/physics/boxCollider.h"
#include "utilities/aStar.h"
#include "utilities/time.h"
#include "game/health.h"

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
	Health health;
	float movementSpeed;
	DirectX::XMVECTOR direction;
	DirectX::XMVECTOR targetRotation;
	const float rotationSpeed = 5.0f;

	// Collider
	std::weak_ptr<BoxCollider> hitbox;

	// Pathfinding
	std::vector<std::shared_ptr<AStarVertex>> path;
	size_t maxPathIndex;
	size_t currentPathIndex;
	bool hasFinishedPath;

	// Shooting
	const float damage = 5.f;
	bool canShoot;
	const float shotCooldown;
	float timeSinceLastShot;

	void MoveAlongPath();
	bool IsAtCurrentPathNode();

	void UpdateShootCooldown();

	void ShootAtCore();
	void ShootAtPlayer();

	void VisualizeRay(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, float distance);
};