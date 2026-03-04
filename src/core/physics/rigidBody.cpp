#include "core/physics/rigidBody.h"
#include "core/physics/collider.h"
#include "core/physics/physicsQueue.h" //here to avoid circular dependencies

RigidBody::RigidBody() {}

RigidBody::~RigidBody() {}

void RigidBody::Start() {
	this->GameObject3D::Start();

	std::weak_ptr<RigidBody> rigidBody = std::static_pointer_cast<RigidBody>(this->GetPtr());
	PhysicsQueue::GetInstance().AddRigidBody(rigidBody);
	Logger::Log("Added Rigidbody to physics queue");

	this->physicsPosition = this->transform.GetPosition();
	this->previousPhysicsPosition = this->physicsPosition;
}

void RigidBody::Tick() {
	this->GameObject3D::Tick();

	// Logger::Log("in update, transform position: " + std::to_string(this->transform.GetPosition().m128_f32[0]) + ", "
	// + std::to_string(this->transform.GetPosition().m128_f32[1]) + ", " +
	// std::to_string(this->transform.GetPosition().m128_f32[2]));
}

void RigidBody::PhysicsTick() {
	this->GameObject3D::PhysicsTick();

	if (this->gravity) this->linearVelocity.y -= 0.6f * Time::GetInstance().GetFixedDeltaTime();
}

void RigidBody::LatePhysicsTick() {
	this->GameObject3D::LatePhysicsTick();

	// the total move for this frame
	DirectX::XMVECTOR moveVector = XMLoadFloat3(&this->linearVelocity);

	DirectX::XMVECTOR collisionCheckPosition = DirectX::XMVectorAdd(this->transform.GetPosition(), moveVector);
	this->transform.SetPosition(collisionCheckPosition); // all velocity has been added, it will first be moved here,
	// then do collision checks to determin the new valid physics position after collision. Then the new current
	// position will be lerped between the last physics pos and this new valid physics pos

	// collision checks should happen after this in sceneManager
}

DirectX::XMVECTOR RigidBody::GetPhysicsPosition() { return this->physicsPosition; }

DirectX::XMVECTOR RigidBody::GetPreviousPhysicsPosition() { return this->previousPhysicsPosition; }

void RigidBody::SetPhysicsPosition(DirectX::XMVECTOR physicsPosition) { this->physicsPosition = physicsPosition; }

void RigidBody::SetPreviousPhysicsPosition(DirectX::XMVECTOR oldPosition) {
	this->previousPhysicsPosition = oldPosition;
}

void RigidBody::SetOnCollisionFunction(std::function<void(std::weak_ptr<GameObject3D>)> function, int index) {
	if (index >= this->colliderChildren.size()) {
		Logger::Log("Tried to set RigidBody collider function with index out of range");
		return;
	}

	if (!this->colliderChildren[index].expired()) {
		this->colliderChildren[index].lock()->SetOnCollision(function);
	}
}

void RigidBody::SetAllOnCollisionFunction(std::function<void(std::weak_ptr<GameObject3D>)> function) {
	for (int i = 0; i < this->colliderChildren.size(); i++) {
		this->SetOnCollisionFunction(function, i);
	}
}

void RigidBody::LoadFromJson(const nlohmann::json& data) {
	this->GameObject3D::LoadFromJson(data);

	if (data.contains("gravity")) {
		this->gravity = static_cast<bool>(data.at("gravity").get<bool>());
		Logger::Log("'gravity' was found in json: " + std::to_string(this->gravity));
	} else {
		Logger::Log("didn't find 'gravity'");
	}
}

void RigidBody::SaveToJson(nlohmann::json& data) {
	this->GameObject3D::SaveToJson(data);

	data["gravity"] = this->gravity;
}

void RigidBody::LateTick() {
	GameObject3D::LatePhysicsTick();

	DirectX::XMFLOAT3 previousPosition;
	DirectX::XMFLOAT3 targetPosition;
	DirectX::XMStoreFloat3(&previousPosition, this->previousPhysicsPosition);
	DirectX::XMStoreFloat3(&targetPosition, this->physicsPosition);

	float lerpProcent = PhysicsQueue::GetInstance().GetFixedDeltaTimeBuffer() / Time::GetInstance().GetFixedDeltaTime();
	DirectX::XMFLOAT3 lerpedPosition = FLOAT3LERP(previousPosition, targetPosition, lerpProcent);
	DirectX::XMVECTOR newPosition = DirectX::XMLoadFloat3(&lerpedPosition);

	this->transform.SetPosition(newPosition);

	// Logger::Log("\n----------------");
	// Logger::Log("transform position: " + std::to_string(this->transform.GetPosition().m128_f32[0]) + ", " +
	// std::to_string(this->transform.GetPosition().m128_f32[1]) + ", " +
	// std::to_string(this->transform.GetPosition().m128_f32[2])); Logger::Log("PREVIOUS PHYS POSITION: " +
	// std::to_string(this->previousPhysicsPosition.m128_f32[0]) + ", " +
	// std::to_string(this->previousPhysicsPosition.m128_f32[1]) + ", " +
	// std::to_string(this->previousPhysicsPosition.m128_f32[2])); Logger::Log("PHYS POSITION: " +
	// std::to_string(this->physicsPosition.m128_f32[0]) + ", " + std::to_string(this->physicsPosition.m128_f32[1]) + ",
	// " + std::to_string(this->physicsPosition.m128_f32[2])); Logger::Log("\n----------------");
}

void RigidBody::SetParent(std::weak_ptr<GameObject> parent) {
	// do we need to add other things?

	if (parent.expired()) {
		Logger::Error("tried setting an expired game object as parent to rigidbody");
		return;
	}

	this->GameObject3D::SetParent(parent);
}

void RigidBody::AddColliderChild(std::weak_ptr<Collider> collider) { this->colliderChildren.push_back(collider); }

void RigidBody::SetId(int id) { this->id = id; }

int RigidBody::GetId() { return this->id; }

int RigidBody::GetNrOfColliderChildren() { return this->colliderChildren.size(); }

std::vector<std::weak_ptr<Collider>>* RigidBody::GetColliderChildrenVector() { return &this->colliderChildren; }

bool RigidBody::Collision(std::weak_ptr<RigidBody> rigidbody, int& nrOfCollisionTestsOnTick) {
	// checking expired pointers should be done before calling this, with RemoveExpiredColliderChildren()

	bool collision = false;
	bool tempCollision = false;

	for (int i = 0; i < this->colliderChildren.size(); i++) {
		Collider* thisCollider = this->colliderChildren[i].lock().get(); // make sure this ptr isn't stored in collider

		for (int j = 0; j < rigidbody.lock()->GetNrOfColliderChildren(); j++)
		{
			Collider* otherCollider = (*rigidbody.lock()->GetColliderChildrenVector())[j].lock().get(); // make sure this ptr isn't stored in collider

			if (thisCollider->GetIgnoreTag() != Tag::DISTANCE && otherCollider->GetIgnoreTag() != Tag::DISTANCE)
			{
				DirectX::XMVECTOR colliderPosition = thisCollider->transform.GetGlobalPosition();
				DirectX::XMVECTOR otherColliderPosition = otherCollider->transform.GetGlobalPosition();
				DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(colliderPosition, otherColliderPosition);

				float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVector));
				float thisExtraCullingDistance = thisCollider->GetExtraCullingDistance();
				float otherExtraCullingDistance = otherCollider->GetExtraCullingDistance();
				float cullingDistance = PhysicsQueue::GetInstance().GetColliderCullingDistanceSquared();

				if (distanceSquared >= cullingDistance + thisExtraCullingDistance + otherExtraCullingDistance) continue; // early exit if colliders are too far apart
			}

			DirectX::XMVECTOR contactNormal = {};
			tempCollision = thisCollider->Collision(otherCollider, contactNormal);

			if (tempCollision)
			{
				collision = true;

				if(!thisCollider->GetSolid() || !otherCollider->GetSolid()) continue;

				//zero out velocity in the opposite direction of contactNormal
				DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&this->linearVelocity);
				float normalComponentMagnitude = DirectX::XMVectorGetX(DirectX::XMVector3Dot(velocity, contactNormal));

				DirectX::XMVECTOR normalComponent = DirectX::XMVectorScale(contactNormal, normalComponentMagnitude);

				// Remove the normal component from the velocity
				DirectX::XMVECTOR newVelocity = DirectX::XMVectorSubtract(velocity, normalComponent);
				DirectX::XMStoreFloat3(&this->linearVelocity, newVelocity);
			}
		}
	}

	return collision;
}

bool RigidBody::Collision(std::weak_ptr<Collider> collider, int& nrOfCollisionTestsOnTick) {
	bool collision = false;
	bool tempCollision = false;
	Collider* otherCollider = collider.lock().get(); // make sure this ptr isn't stored in collider

	if (collider.expired()) {
		Logger::Log("RigidBody tried colliding with null-collider");
		return false;
	}

	for (int i = this->colliderChildren.size() - 1; i >= 0; i--) {
		if (this->colliderChildren[i].expired()) {
			Logger::Log("RigidBody had null-collider child, removing from RigidBody");
			this->colliderChildren.erase(this->colliderChildren.begin() + i);
			continue;
		}

		Collider* thisCollider = this->colliderChildren[i].lock().get(); // make sure this ptr isn't stored in collider

		if (thisCollider->GetIgnoreTag() != Tag::DISTANCE && otherCollider->GetIgnoreTag() != Tag::DISTANCE)
		{
			DirectX::XMVECTOR colliderPosition = thisCollider->transform.GetGlobalPosition();
			DirectX::XMVECTOR otherColliderPosition = otherCollider->transform.GetGlobalPosition();
			DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(colliderPosition, otherColliderPosition);

			float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVector));
			float thisExtraCullingDistance = thisCollider->GetExtraCullingDistance();
			float otherExtraCullingDistance = otherCollider->GetExtraCullingDistance();
			float cullingDistance = PhysicsQueue::GetInstance().GetColliderCullingDistanceSquared();

			if (distanceSquared >= cullingDistance + thisExtraCullingDistance + otherExtraCullingDistance) continue;
		}

		DirectX::XMVECTOR contactNormal = {};
		tempCollision = thisCollider->Collision(otherCollider, contactNormal); // doesnt check nor of ticks right now

		if (tempCollision)
		{
			collision = true;

			if(!thisCollider->GetSolid() || !otherCollider->GetSolid()) continue;

			//zero out velocity in the opposite direction of contactNormal
			DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&this->linearVelocity);
			float normalComponentMagnitude = DirectX::XMVectorGetX(DirectX::XMVector3Dot(velocity, contactNormal));

			DirectX::XMVECTOR normalComponent = DirectX::XMVectorScale(contactNormal, normalComponentMagnitude);

			// Remove the normal component from the velocity
			DirectX::XMVECTOR newVelocity = DirectX::XMVectorSubtract(velocity, normalComponent);
			DirectX::XMStoreFloat3(&this->linearVelocity, newVelocity);
		}
	}

	return collision;
}