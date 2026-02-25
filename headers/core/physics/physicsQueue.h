#pragma once
#include "core/physics/rigidBody.h"
#include "core/physics/rayCaster.h"
#include <limits>

class PhysicsQueue
{
public:
	PhysicsQueue(PhysicsQueue& other) = delete;
	PhysicsQueue& operator=(const PhysicsQueue&) = delete;

	static PhysicsQueue& GetInstance();

    int GetPhysicsTickCounter();
    void ResetPhysicsTickCounter();
    float GetFixedDeltaTimeBuffer();

    void Tick();

    /// <summary>
    /// Should be called after collision checks and resolutions have been done so all rigidbodies gets their new valid physics position
    /// </summary>
    void UpdatePhysicsPositions();

    /// <summary>
    /// Adds RigidBody to PhysicsQueue to check and resolve collisions
    /// </summary>
    /// <param name="rigidBody"></param>
    void AddRigidBody(std::weak_ptr<RigidBody> rigidBody);

    /// <summary>
    /// Adds Collider with GameObject3D parent or no parent at all to PhysicsQueue to check and resolve collisions
    /// </summary>
    /// <param name="collider"></param>
    void AddStrayCollider(std::weak_ptr<Collider> collider);

    /// <summary>
    /// Adds collider to the array with all existing colliders
    /// </summary>
    /// <param name="collider"></param>
    void AddToAllColliders(std::weak_ptr<Collider> collider);

    /// <summary>
    /// Sets the distance at which collision checks stops being performed
    /// </summary>
    /// <param name="distanceSquared"></param>
    void SetColliderCullingDistanceSquared(float distanceSquared);

    /// <summary>
    /// Gets the distance at which collision checks stops being performed
    /// </summary>
    /// <param name="distanceSquared"></param>
    float GetColliderCullingDistanceSquared();

    /// <summary>
    /// Starts checking and resolving current collisions
    /// </summary>
    void SolveCollisions();
    /// <summary>
    /// casts ray with origin point and direction aswell as optional max distance,
    /// populates rayCastData struct with hit object and distance to hit
    /// </summary>
    /// <param name="ray"></param>
    /// <param name="rayCastData"></param>
    /// <param name="maxDistance"></param>
    /// <returns></returns>
	bool castRay(Ray& ray, RayCastData& rayCastData, size_t targetTag = Tag::NOIGNORE, size_t tagsToIgnore = Tag::NOIGNORE, float maxDistance = (std::numeric_limits<float>::max)());

private:
    PhysicsQueue();
    ~PhysicsQueue();

    int nrOfCollisionTestOnTick = 0;
    float colliderCullingDistanceSquared = 9;

    int rigidBodyIdCounter = 0;
    int colliderIdCounter = 0;

    float fixedDeltaTimeBuffer = 0;
    float physicsTickCounter = 0;

    std::vector<std::weak_ptr<RigidBody>> rigidBodies;
    std::vector<std::weak_ptr<Collider>> allColiders;
	std::vector<std::weak_ptr<Collider>> strayColliders;

    RayCaster rayCaster;
};