#pragma once

#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include <vector>

class Player;

class Turret : public MeshObject {
public:
	virtual void Start() override;
	virtual void Tick() override;

	/// <summary>
	/// Sets the turrets target
	/// </summary>
	void SetTarget(std::weak_ptr<GameObject3D> target);

	/// <summary>
	/// Sets the turret to target the closest entity in potentialTargets
	/// </summary>
	void SetTargetClosest();

	/// <summary>
	/// Sets how many rounds per minute are fired.
	/// </summary>
	void SetRPM(float rpm);
	void SetRange(float range);
	void SetDamage(float damage);

	bool HasTarget() const;

	std::weak_ptr<GameObject3D> GetTarget() const;
	virtual void Fire();

private:
	void RemoveInteract(std::shared_ptr<Player> player);
	void HoverRemove();

	float lastAttemptedTargeting = 0;
	float retargetTime = 0.5;

	void SetDirection(DirectX::XMVECTOR newDirection);

	SoundClip* shootSound;
	std::weak_ptr<SoundSourceObject> speaker;
	std::weak_ptr<GameObject3D> target;

	std::weak_ptr<GameObject3D> turretPart;
	std::weak_ptr<GameObject3D> framePart;

	float turnSpeedRPS = 3.14;
	float lastFired = 0;
	float rpm = 60;
	float range = 20;
	float damage = 20;
};