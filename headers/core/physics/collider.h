#pragma once
#include "core/physics/physics.h"
#include "core/physics/ray.h"
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include <functional>
#include <memory>


static const DirectX::XMFLOAT3 localBoxCorners[8] = {{-1, -1, -1}, {+1, -1, -1}, {-1, +1, -1}, {+1, +1, -1},
													 {-1, -1, +1}, {+1, -1, +1}, {-1, +1, +1}, {+1, +1, +1}};
/// <summary>
/// Tag is used to tell what the collider is for, and will in combination with ignoreTag minimize the nr of collision checks
/// </summary>
/// <returns></returns>
enum Tag 
{
    PLAYER         = 1 << 0,  // 1
    PLAYER_ATTACK  = 1 << 1,  // 2
    ENEMY          = 1 << 2,  // 4
    ENEMY_ATTACK   = 1 << 3,  // 8
    GROUND         = 1 << 4,  // 16
    FLOOR          = 1 << 5,  // 32
    OBJECT         = 1 << 6,  // 64
    WALL           = 1 << 7,  // 128
    INTERACTABLE   = 1 << 8,  // 256
    DISTANCE       = 1 << 9,  // 512
    NOIGNORE       = 1 << 10  // 1024
 };

enum ColliderType { BOX, SPHERE, NONE };

class BoxCollider; // forward declaration
class SphereCollider;
class RigidBody;
class Player;

class Collider : public GameObject3D {
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
	bool Collision(Collider* other, int& nrOfCollisionTestsOnTick);

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

	void SetExtraCullingDistance(float distanceSquared);
	float GetExtraCullingDistance();

	/// <summary>
	/// returns true if ray intersects object with distance
	/// to hit and max ray distance
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="distance"></param>
	/// <returns></returns>
	virtual bool IntersectWithRay(const Ray& ray, float& distance, float maxDistance) = 0;

	void Interact(std::shared_ptr<Player> player) { this->interactFunc(player); };
	void Hover() { this->hoverFunc(); };
	void Hit(float damage) { this->hitFunc(damage); }
	void OnCollision(std::weak_ptr<GameObject3D> gameObject3D) { this->collisionFunc(gameObject3D); }

	void SetOnInteract(std::function<void(std::shared_ptr<Player>)> func) { this->interactFunc = func; }
	void SetOnHover(std::function<void()> func) { this->hoverFunc = func; }
	void SetOnHit(std::function<void(float)> func) { this->hitFunc = func; }
	void SetOnCollision(std::function<void(std::weak_ptr<GameObject3D>)> func) { this->collisionFunc = func; }

	void SetType(ColliderType type);
	ColliderType GetType();
	size_t GetTag();
	void SetTag(size_t tag);
	size_t GetIgnoreTag();
	void SetIgnoreTag(size_t ignoreTag);
	bool GetSolid();
	void SetSolid(bool solid);
	bool GetDynamic();
	void SetDynamic(bool dynamic);

private:
	ColliderType type = ColliderType::NONE;
	size_t tag = Tag::OBJECT;
	size_t ignoreTag = Tag::NOIGNORE;
	PhysicsMaterial physicsMaterial;
	bool solid = true;
	bool dynamic = false;
	bool hasInitializedPreviousPosition = false;

	float extraCullingDistanceSquared = 0;

	std::function<void(std::shared_ptr<Player>)> interactFunc = [](std::shared_ptr<Player>) {};
	std::function<void()> hoverFunc = []() {};
	std::function<void(float)> hitFunc = [](float) {};
	std::function<void(std::weak_ptr<GameObject3D>)> collisionFunc = [](std::weak_ptr<GameObject3D>) {};

	int id = -1;
	std::weak_ptr<GameObject> meshObjectChild; // reference to the mesh visual representation of the collider (remove?)
	std::weak_ptr<RigidBody> rigidBodyParent;
	std::weak_ptr<GameObject3D> gameObject3DParent;

	bool CollisionHandling(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance);
};