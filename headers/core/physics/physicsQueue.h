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
    bool castRay(Ray& ray, RayCastData& rayCastData, float maxDistance = (std::numeric_limits<float>::max)());

private:
    PhysicsQueue();
    ~PhysicsQueue();

    int rigidBodyIdCounter = 0;
    int colliderIdCounter = 0;

    std::vector<std::weak_ptr<RigidBody>> rigidBodies;
    std::vector<std::weak_ptr<Collider>> allColiders;
	std::vector<std::weak_ptr<Collider>> strayColliders;


    RayCaster rayCaster;
};