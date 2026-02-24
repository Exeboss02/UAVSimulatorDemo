#pragma once

#include "gameObjects/meshObject.h"
#include "gameObjects/gameObject3D.h"
#include <vector>

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
	void SetTargetClosest(const std::vector<std::weak_ptr<GameObject3D>> potentialTargets);

	/// <summary>
	/// Sets how many rounds per minute are fired.
	/// </summary>
	void SetRPM(float rpm);
	void SetRange(float range);
	void SetDamage(float damage);

	bool HasTarget() const;

	std::weak_ptr<GameObject3D> GetTarget() const;
	void Fire();
private:
	std::weak_ptr<GameObject3D> target;
	float lastFired = 0;
	float rpm = 60;
	float range = 20;
	float damage = 10;

};