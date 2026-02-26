#pragma once

#include <memory>
#include <vector>

#include "core/physics/boxCollider.h"
#include "game/health.h"
#include "gameObjects/gameObject3D.h"
#include "utilities/aStar.h"
#include "utilities/time.h"

enum MoveSpeedMode { NORMAL, SLOWED };

class Enemy : public GameObject3D {
public:
	Enemy();
	~Enemy() = default;
	void Start() override;
	void Tick() override;

	void SlowDownEnemy(float durationInSec);

	void SetPath(const std::vector<std::shared_ptr<AStarVertex>>& newPath);

	void KillSelf();

	void DecrementHealth(size_t amount);

	void IncrementHealth(size_t amount);

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
	void UpdateShootCooldown();

	// Slow effect
	bool isSlowed;
	float slowDuration;
	float timeSinceSlowed;
	void UpdateSlowEffect();

	void SetMoveSpeedMode(MoveSpeedMode mode);
	void MoveAlongPath();
	bool IsAtCurrentPathNode();

	void ShootAtCore();
	void ShootAtPlayer();

	void VisualizeRay(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, float distance);
};