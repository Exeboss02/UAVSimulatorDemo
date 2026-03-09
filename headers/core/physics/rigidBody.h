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
	DirectX::XMFLOAT3 angularVelocity = {};

	void Start() override;
	void Tick() override;
	void LateTick() override;
	void PhysicsTick() override;
	void LatePhysicsTick() override;

	DirectX::XMVECTOR GetPhysicsPosition();
	DirectX::XMVECTOR GetPreviousPhysicsPosition();
	void SetPhysicsPosition(DirectX::XMVECTOR physicsPosition);
	void SetPreviousPhysicsPosition(DirectX::XMVECTOR oldPosition);

	/// @brief Sets which function that should be called when a collider at index on the RigidBody collides with another collider
	/// @param function 
	/// @param index 
	void SetOnCollisionFunction(std::function<void(std::weak_ptr<GameObject3D>, std::weak_ptr<Collider> collider)> function, int index);

	/// @brief Sets which function that should be called when any collider on the RigidBody collides with another collider
	/// @param function 
	/// @param index 
	void SetAllOnCollisionFunction(std::function<void(std::weak_ptr<GameObject3D>, std::weak_ptr<Collider>)> function);

	void LoadFromJson(const nlohmann::json& data) override;
	void SaveToJson(nlohmann::json& data) override;

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
	bool Collision(std::weak_ptr<RigidBody> rigidbody, int& nrOfCollisionTestsOnTick);

	/// <summary>
	/// Checks and resolves collision with other Collider according to collider children's settings
	/// </summary>
	/// <param name="collider"></param>
	/// <returns></returns>
	bool Collision(std::weak_ptr<Collider> collider, int& nrOfCollisionTestsOnTick);


private:
	std::vector<std::weak_ptr<Collider>> colliderChildren;
	int id = -1;

	DirectX::XMVECTOR physicsPosition = {};
	DirectX::XMVECTOR previousPhysicsPosition = {};
};