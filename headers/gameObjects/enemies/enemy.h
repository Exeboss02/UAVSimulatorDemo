#pragma once

#include <memory>
#include <vector>

#include "core/physics/boxCollider.h"
#include "game/health.h"
#include "gameObjects/gameObject3D.h"
#include "utilities/aStar.h"
#include "utilities/time.h"
#include "core/tools.h"

#include "gameObjects/meshObject.h"

enum MoveSpeedMode { NORMAL, SLOWED, FAST };

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

protected:
	Health health;
	float movementSpeed;
	DirectX::XMVECTOR direction;
	DirectX::XMVECTOR bodyTargetRotation;
	float rotationSpeed;

	// Collider
	std::weak_ptr<SphereCollider> hitbox;

	std::weak_ptr<GameObject3D> head;
	DirectX::XMVECTOR headOffsetFromBody = DirectX::XMVectorSet(0, 0, 0, 0);

	// Pathfinding
	std::vector<std::shared_ptr<AStarVertex>> path = {};
	size_t maxPathIndex = 0;
	size_t currentPathIndex = 0;
	bool hasFinishedPath = false;

	std::weak_ptr<SoundSourceObject> speaker;

	// Shooting
	float damage = 5.f;
	float shootRange = 30.f;
	bool canShoot = true;
	float shotCooldown;
	float timeSinceLastShot = 0.f;
	float playerHitAccuracy;
	void UpdateShootCooldown(const float deltaTime);

	// Slow effect
	bool isSlowed = false;
	float slowDuration = 0.f;
	float timeSinceSlowed = 0.f;
	void UpdateSlowEffect(const float deltaTime);

	virtual void SetMoveSpeedMode(MoveSpeedMode mode) = 0;
	void MoveAlongPath(const float deltaTime);
	bool IsAtCurrentPathNode();

	void CalulateDirectionToTarget();
	void CalulateTargetRotation();

	// Stuck detection
	float timeStuck;
	float stuckCheckInterval;
	void IsStuckOnPath(const float deltaTime);

	void ShootAtCore();
	virtual bool ShootAtPlayer(const float deltaTime) = 0;

	void RotateBody(const float deltaTime);
	void RotateHead(DirectX::XMVECTOR& newDirection, const float deltaTime);

	void VisualizeRay(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, float distance);
};