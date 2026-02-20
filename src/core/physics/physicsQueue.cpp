#include "core/physics/physicsQueue.h"

PhysicsQueue::~PhysicsQueue()
{
}

void PhysicsQueue::UpdatePhysicsPositions()
{
    for(int i = 0; i < this->rigidBodies.size(); i++)
    {
        std::shared_ptr<RigidBody> rigidBody = this->rigidBodies[i].lock();
        if(!rigidBody) continue;

        rigidBody->SetPreviousPhysicsPosition(rigidBody->GetPhysicsPosition());
        rigidBody->SetPhysicsPosition(rigidBody->transform.GetPosition());
    }
}

PhysicsQueue::PhysicsQueue()
{
}

PhysicsQueue& PhysicsQueue::GetInstance()
{
	static PhysicsQueue instance;
	return instance;
}

int PhysicsQueue::GetPhysicsTickCounter() 
{
    return this->physicsTickCounter; 
}

void PhysicsQueue::ResetPhysicsTickCounter()
{
    this->physicsTickCounter = 0;
}

float PhysicsQueue::GetFixedDeltaTimeBuffer() 
{
    return this->fixedDeltaTimeBuffer;
}

void PhysicsQueue::Tick()
{
    this->fixedDeltaTimeBuffer += Time::GetInstance().GetDeltaTime();
    float fixedDeltaTime = Time::GetInstance().GetFixedDeltaTime();

	while (this->fixedDeltaTimeBuffer >= fixedDeltaTime)
	{
		this->fixedDeltaTimeBuffer -= fixedDeltaTime;
		this->physicsTickCounter++;
	}

    if(this->physicsTickCounter > 5)
    {
        this->physicsTickCounter = 6;
        this->fixedDeltaTimeBuffer = 0;
    }

    //Logger::Log("Nr of phsyics ticks: " + std::to_string(this->physicsTickCounter));
}

void PhysicsQueue::AddRigidBody(std::weak_ptr<RigidBody> rigidBody) {
	rigidBody.lock()->SetId(this->rigidBodyIdCounter);
    this->rigidBodies.push_back(rigidBody);

    this->rigidBodyIdCounter++;
}

void PhysicsQueue::AddStrayCollider(std::weak_ptr<Collider> collider)
{
    collider.lock()->SetId(this->colliderIdCounter);
    this->strayColliders.push_back(collider);

    this->colliderIdCounter++;
}
void PhysicsQueue::AddToAllColliders(std::weak_ptr<Collider> collider) {
	collider.lock()->SetId(this->colliderIdCounter);
	this->allColiders.push_back(collider);

	this->colliderIdCounter++;
}

void PhysicsQueue::SetColliderCullingDistanceSquared(float distanceSquared)
{
    this->colliderCullingDistanceSquared = distanceSquared;
}

float PhysicsQueue::GetColliderCullingDistanceSquared() 
{ 
    return this->colliderCullingDistanceSquared; 
}

void PhysicsQueue::SolveCollisions() {
	//RigidBody vs RigidBody collisions
    for(int i = rigidBodies.size() - 1; i >= 0; i--)
    {
        std::shared_ptr<RigidBody> thisRigidBody = this->rigidBodies[i].lock();
        if(!thisRigidBody)
        {
            this->rigidBodies.erase(this->rigidBodies.begin() + i);
            continue;
        }

        DirectX::XMVECTOR rigidBodyPosition = thisRigidBody->transform.GetPosition();

        for (int j = i - 1; j >= 0; j--)
        {
            std::shared_ptr<RigidBody> otherRigidBody = this->rigidBodies[j].lock();
            if(!otherRigidBody)
            {
                this->rigidBodies.erase(this->rigidBodies.begin() + j);
                i--;
                continue;
            }

            DirectX::XMVECTOR otherRigidBodyPosition = otherRigidBody->transform.GetPosition();
            DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(rigidBodyPosition, otherRigidBodyPosition);
            float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVector));

            if(distanceSquared >= this->colliderCullingDistanceSquared) continue; //early exit if colliders are too far apart

            thisRigidBody->Collision(otherRigidBody, this->nrOfCollisionTestOnTick);
        }
    }

    //StrayCollider vs StrayCollider collisions
    for(int i = strayColliders.size() - 1; i >= 0; i--)
    {
        std::shared_ptr<Collider> thisCollider = this->strayColliders[i].lock();
        if(!thisCollider)
        {
            this->strayColliders.erase(this->strayColliders.begin() + i);
            continue;
        }

        DirectX::XMVECTOR colliderPosition = thisCollider->transform.GetPosition();

        for (int j = i - 1; j >= 0; j--)
        {
            std::shared_ptr<Collider> otherCollider = this->strayColliders[j].lock();
            if(!otherCollider)
            {
                this->strayColliders.erase(this->strayColliders.begin() + j);
                i--;
                continue;
            }

            DirectX::XMVECTOR otherColliderPosition = otherCollider->transform.GetPosition();
            DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(colliderPosition, otherColliderPosition);
            float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVector));

            if(distanceSquared >= this->colliderCullingDistanceSquared) continue; //early exit if colliders are too far apart

            thisCollider->Collision(otherCollider.get(), this->nrOfCollisionTestOnTick);
        }
    }

    //RigidBody vs StrayCollider collisions
    for(int i = rigidBodies.size() - 1; i >= 0; i--)
    {
        std::shared_ptr<RigidBody> rigidBody = this->rigidBodies[i].lock();
        if(!rigidBody)
        {
            this->rigidBodies.erase(this->rigidBodies.begin() + i);
            continue;
        }

        DirectX::XMVECTOR rigidBodyPosition = rigidBody->transform.GetPosition();

        for (int j = strayColliders.size() - 1; j >= 0; j--)
        {
            std::shared_ptr<Collider> collider = this->strayColliders[j].lock();
            if(!collider)
            {
                this->strayColliders.erase(this->strayColliders.begin() + j);
                i--;
                continue;
            }

            DirectX::XMVECTOR colliderPosition = collider->transform.GetPosition();
            DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(rigidBodyPosition, colliderPosition);
            float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVector));

            if(distanceSquared >= this->colliderCullingDistanceSquared) continue; //early exit if colliders are too far apart

            rigidBody->Collision(collider, this->nrOfCollisionTestOnTick);
        }
    }

    Logger::Log("nr of collision tests: ", std::to_string(this->nrOfCollisionTestOnTick));
    this->nrOfCollisionTestOnTick = 0;
}

bool PhysicsQueue::castRay(Ray& ray, RayCastData& rayCastData, float maxDistance) {
	//not ok but done elsewhere so if fps no good, fix this everywhere 
    for (int i = allColiders.size() - 1; i >= 0; i--) {
		if (this->allColiders[i].expired()) {
			this->allColiders.erase(this->allColiders.begin() + i);
		}
	}
   return this->rayCaster.castRay(ray, rayCastData, this->allColiders, maxDistance);
}