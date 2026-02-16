#pragma once
#include <memory>
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/physics/physics.h"
#include "core/physics/ray.h"

static const DirectX::XMFLOAT3 localBoxCorners[8] =
{
	{ -1, -1, -1 },
	{ +1, -1, -1 },
	{ -1, +1, -1 },
	{ +1, +1, -1 },
	{ -1, -1, +1 },
	{ +1, -1, +1 },
	{ -1, +1, +1 },
	{ +1, +1, +1 }
};



enum Tag
{
	PLAYER,
	PLAYER_ATTACK,
	ENEMY,
	ENEMY_ATTACK,
	GROUND,
	FLOOR,
	OBJECT,
};

enum ColliderType
{
	BOX,
	SPHERE,
	NONE
};

class BoxCollider; //forward declaration
class SphereCollider;
class RigidBody;

class Collider : public GameObject3D
{
public:
	Collider();
	~Collider();

	float shortestExtent = 1;

	virtual void Tick() override;
	void SetId(int id);
	int GetId();

	/// <summary>
	/// ONLY USED BY ENGINE - Resolves collision within a colliding pair
	/// </summary>
	/// <param name="resolveAxis"></param>
	/// <param name="resolveDistance"></param>
	void ResolveCollision(DirectX::XMFLOAT3 resolveAxis, float resolveDistance);

	/// <summary>
	/// USE THIS - Checks for collisions and automatically resolves them based on collider settings
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	bool Collision(Collider* other);

	/// <summary>
	/// USE THIS - Checks for collisions and automatically resolves them based on collider settings
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	bool Collision(Collider* other, DirectX::XMVECTOR& contactNormal);

	/// <summary>
	/// Sets parent for collider
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(std::weak_ptr<GameObject> parent) override;
	void Start() override;

	/// <summary>
	/// ONLY USED BY ENGINE - Starts Double dispatch call chain, automatically determines collider types
	/// </summary>
	/// <param name="resolveAxis"></param>
	/// <param name="resolveDistance"></param>
	virtual bool DoubleDispatchCollision(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance) = 0;

	/// <summary>
	/// ONLY USED BY ENGINE - When a collider collides with a box collider
	/// </summary>
	/// <param name="resolveAxis"></param>
	/// <param name="resolveDistance"></param>
	virtual bool CollidesWithBox(BoxCollider* box, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance) = 0;

	/// <summary>
	/// ONLY USED BY ENGINE - When a collider collides with a sphere collider
	/// </summary>
	/// <param name="resolveAxis"></param>
	/// <param name="resolveDistance"></param>
	virtual bool CollidesWithSphere(SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance) = 0;

	/// <summary>
	/// Sets collider position
	/// </summary>
	/// <param name="resolveAxis"></param>
	/// <param name="resolveDistance"></param>
	virtual void SetPosition(DirectX::XMFLOAT3 newPosition) = 0;

	/// <summary>
	/// ONLY USED BY ENGINE - Collision check between a box and sphere collider
	/// </summary>
	/// <param name="resolveAxis"></param>
	/// <param name="resolveDistance"></param>
	bool BoxSphereCollision(BoxCollider* box, SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance);

	//DirectX::XMFLOAT3 resolveAxis = {};
	//float resolveDistance = 0;

	ColliderType type = ColliderType::NONE;
	Tag tag = Tag::OBJECT;
	Tag targetTag = Tag::OBJECT;
	PhysicsMaterial physicsMaterial;
	//DirectX::XMFLOAT3 previousPosition = {};
	bool solid = true;
	bool dynamic = false;
	bool hasInitializedPreviousPosition = false;

	/// <summary>
	/// returns true if ray intersects object with distance 
	/// to hit and max ray distance 
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="distance"></param>
	/// <returns></returns>
	virtual bool IntersectWithRay(const Ray& ray, float& distance, float maxDistance) = 0;

private:
	int id = -1;
	std::weak_ptr<GameObject> meshObjectChild; //reference to the mesh visual representation of the collider (remove?)
	std::weak_ptr<RigidBody> rigidBodyParent;
	std::weak_ptr<GameObject3D> gameObject3DParent;

	bool CollisionHandling(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance);
};