#pragma once
#include "gameObjects/gameObject3D.h"
#include "core/physics/collision.h"

class Collider;

class RigidBody : public GameObject3D
{
public:
	RigidBody();
	~RigidBody();

	bool gravity = true;
	DirectX::XMFLOAT3 linearVelocity = {};

	void Start() override;
	void Tick() override;
	void LateTick() override;
	void PhysicsTick() override;
	void LatePhysicsTick() override;

	/// <summary>
	/// Sets parent
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(std::weak_ptr<GameObject> parent) override;

	/// <summary>
	/// Adds a child of type collider to RigidBody's exclusive collider array
	/// </summary>
	/// <param name="collider"></param>
	void AddColliderChild(std::weak_ptr<Collider> collider);

	/// <summary>
	/// Sets RigidBody id
	/// </summary>
	/// <param name="id"></param>
	void SetId(int id);

	/// <summary>
	/// Retrieves RigidBody id
	/// </summary>
	/// <returns></returns>
	int GetId();

	/// <summary>
	/// Retrieve number of collider children
	/// </summary>
	/// <returns></returns>
	int GetNrOfColliderChildren();

	/// <summary>
	/// Retrieve the vector reference to collider children
	/// </summary>
	/// <returns></returns>
	std::vector<std::weak_ptr<Collider>>* GetColliderChildrenVector();

	/// <summary>
	/// Checks and resolves collision with other RigidBody according to collider children's settings
	/// </summary>
	/// <param name="rigidbody"></param>
	/// <returns></returns>
	bool Collision(std::weak_ptr<RigidBody> rigidbody);

	/// <summary>
	/// Checks and resolves collision with other Collider according to collider children's settings
	/// </summary>
	/// <param name="collider"></param>
	/// <returns></returns>
	bool Collision(std::weak_ptr<Collider> collider);


private:
	std::vector<std::weak_ptr<Collider>> colliderChildren;
	int id = -1;
};