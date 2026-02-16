#include "core/physics/rigidBody.h"
#include "core/physics/collider.h"
#include "core/physics/physicsQueue.h" //here to avoid circular dependencies

RigidBody::RigidBody()
{
}

RigidBody::~RigidBody()
{
}

void RigidBody::Start()
{
	this->GameObject3D::Start();

	std::weak_ptr<RigidBody> rigidBody = std::static_pointer_cast<RigidBody>(this->GetPtr());
	PhysicsQueue::GetInstance().AddRigidBody(rigidBody);
	Logger::Log("Added Rigidbody to physics queue");

	this->physicsPosition = this->transform.GetPosition();
	this->previousPhysicsPosition = this->physicsPosition;
}

void RigidBody::Tick()
{
	this->GameObject3D::Tick();
}

void RigidBody::PhysicsTick()
{
	this->GameObject3D::PhysicsTick();

	if(this->gravity) this->linearVelocity.y -= 6.0f * Time::GetInstance().GetDeltaTime();

	this->previousPhysicsPosition = this->physicsPosition;

	Logger::Log("PREVIOUS PHYS POSITION: " + std::to_string(this->previousPhysicsPosition.m128_f32[0]) + ", " + std::to_string(this->previousPhysicsPosition.m128_f32[1]) + ", " + std::to_string(this->previousPhysicsPosition.m128_f32[2]));
	Logger::Log("PHYS POSITION: " + std::to_string(this->physicsPosition.m128_f32[0]) + ", " + std::to_string(this->physicsPosition.m128_f32[1]) + ", " + std::to_string(this->physicsPosition.m128_f32[2]));
}

void RigidBody::LatePhysicsTick()
{
	this->GameObject3D::LatePhysicsTick();

	//the total move for this frame
	DirectX::XMVECTOR moveVector = XMLoadFloat3(&this->linearVelocity);
	this->physicsPosition = DirectX::XMVectorAdd(this->transform.GetPosition(), moveVector);

	Logger::Log("linear velocity: " + std::to_string(this->linearVelocity.x) + ", " + std::to_string(this->linearVelocity.y) + ", " + std::to_string(this->linearVelocity.z));

	this->transform.SetPosition(this->previousPhysicsPosition);

	//collision checks should happen after this in sceneManager
}

void RigidBody::LateTick()
{
	GameObject3D::LatePhysicsTick();

	DirectX::XMFLOAT3 previousPosition;
	DirectX::XMFLOAT3 targetPosition;
	DirectX::XMStoreFloat3(&previousPosition, this->previousPhysicsPosition);
	DirectX::XMStoreFloat3(&targetPosition, this->physicsPosition);

	float lerpProcent = PhysicsQueue::GetInstance().GetFixedDeltaTimeBuffer() / Time::GetInstance().GetFixedDeltaTime();
	DirectX::XMFLOAT3 lerpedPosition = FLOAT3LERP(previousPosition, targetPosition, lerpProcent);
	DirectX::XMVECTOR newPosition = DirectX::XMLoadFloat3(&lerpedPosition);

	//this->transform.SetPosition(newPosition);
}

void RigidBody::SetParent(std::weak_ptr<GameObject> parent) {
	//do we need to add other things?
	
	if(parent.expired())
	{
		Logger::Error("tried setting an expired game object as parent to rigidbody");
		return;
	}

	this->GameObject3D::SetParent(parent);
}

void RigidBody::AddColliderChild(std::weak_ptr<Collider> collider)
{
	this->colliderChildren.push_back(collider);
}

void RigidBody::SetId(int id)
{
	this->id = id;
}

int RigidBody::GetId()
{ 
	return this->id; 
}

int RigidBody::GetNrOfColliderChildren()
{
	return this->colliderChildren.size();
}

std::vector<std::weak_ptr<Collider>>* RigidBody::GetColliderChildrenVector()
{
	return &this->colliderChildren;
}

bool RigidBody::Collision(std::weak_ptr<RigidBody> rigidbody)
{
	//checking expired pointers should be done before calling this, with RemoveExpiredColliderChildren()

	bool collision = false;
	bool tempCollision = false;

	for (int i = 0; i < this->colliderChildren.size(); i++)
	{
		Collider* thisCollider = this->colliderChildren[i].lock().get(); //make sure this ptr isn't stored in collider

		for (int j = 0; j < rigidbody.lock()->GetNrOfColliderChildren(); j++)
		{
			Collider* otherCollider = (*rigidbody.lock()->GetColliderChildrenVector())[j].lock().get(); //make sure this ptr isn't stored in collider
			tempCollision = thisCollider->Collision(otherCollider);

			if (tempCollision) collision = true;
		}
	}

	return collision;
}

bool RigidBody::Collision(std::weak_ptr<Collider> collider)
{
	bool collision = false;
	bool tempCollision = false;
	Collider* otherCollider = collider.lock().get(); //make sure this ptr isn't stored in collider

	if(collider.expired())
	{
		Logger::Log("RigidBody tried colliding with null-collider");
		return false;
	}

	for (int i = this->colliderChildren.size() - 1; i >= 0; i--)
	{
		if(this->colliderChildren[i].expired())
		{
			Logger::Log("RigidBody had null-collider child, removing from RigidBody");
			this->colliderChildren.erase(this->colliderChildren.begin() + i);
			continue;
		}

		Collider* thisCollider = this->colliderChildren[i].lock().get(); //make sure this ptr isn't stored in collider

		tempCollision = thisCollider->Collision(otherCollider);
		if (tempCollision) collision = true;
	}

	return collision;
}
