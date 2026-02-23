#include "gameObjects/enemy.h"

#include "core/physics/vector3D.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "core/physics/physicsQueue.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/meshObjData.h"
#include "core/assetManager.h"

Enemy::Enemy() : GameObject3D(), health(100), path({}), maxPathIndex(0), currentPathIndex(0),
	hasFinishedPath(false), timeSinceLastShot(0.0f)
{
	this->direction = DirectX::XMVectorSet(0, 0, 1, 0);
	this->SetMoveSpeedMode(MoveSpeedMode::NORMAL);
}

void Enemy::Start() { 
	auto meshObj = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	meshObj->SetParent(this->GetPtr());

	MeshObjData meshData = AssetManager::GetInstance().GetMeshObjData(""); // TODO: Add enemy mesh
	meshObj->SetMesh(meshData);

	auto collider = this->factory->CreateGameObjectOfType<BoxCollider>().lock();
	collider->transform.SetScale({1, 1, 1});
	collider->SetParent(this->GetPtr());

	collider->SetOnInteract([&]() {
		this->health -= 20;
		if (this->health <= 0) {
			this->factory->QueueDeleteGameObject(this->GetPtr());
		}
	});
	this->hitbox = collider;
}

void Enemy::Tick() { 
	if (this->maxPathIndex != 0 && !this->hasFinishedPath) {
		this->MoveAlongPath();
	}

	// Reached cockpit and can now attack core
	else if (this->hasFinishedPath) {
		this->ShootAtCore();
	}
}

void Enemy::SetMoveSpeedMode(MoveSpeedMode mode) {
	switch (mode) {
		case MoveSpeedMode::NORMAL:
			this->movementSpeed = 2.0f;
			break;
		case MoveSpeedMode::SLOWED:
			this->movementSpeed = 1.0f;
			break;
		default:
			this->movementSpeed = 2.0f;
			break;
	}
}

void Enemy::SetPath(const std::vector<std::shared_ptr<AStarVertex>>& newPath) {
	this->path = newPath;
	this->maxPathIndex = newPath.size() - 1;
	this->currentPathIndex = 0;
	this->hasFinishedPath = false;
	this->transform.SetPosition(this->path[0]->transform.GetGlobalPosition());
}

void Enemy::MoveAlongPath() { 
	if (this->currentPathIndex >= this->maxPathIndex - 1) { // Stop on node before core.
		this->hasFinishedPath = true;
		return;
	}

	if (this->IsAtCurrentPathNode()) {
		this->currentPathIndex++;
		this->direction = DirectX::XMVectorSubtract(this->path[this->currentPathIndex]->transform.GetGlobalPosition(),
													this->transform.GetGlobalPosition());
		this->direction = DirectX::XMVector3Normalize(this->direction);

		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, this->direction);
		right = DirectX::XMVector3Normalize(right);
		DirectX::XMVECTOR actualUp = DirectX::XMVector3Cross(this->direction, right);

		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();
		rotationMatrix.r[0] = right;
		rotationMatrix.r[1] = actualUp;
		rotationMatrix.r[2] = this->direction;
		rotationMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationMatrix(rotationMatrix);
		this->transform.SetRotationQuaternion(rotation);
	}

	this->transform.Move(this->direction, this->movementSpeed * Time::GetInstance().GetDeltaTime());
}

bool Enemy::IsAtCurrentPathNode() { 
	return DirectX::XMVector4NearEqual(this->transform.GetGlobalPosition(),
										this->path[this->currentPathIndex]->transform.GetGlobalPosition(),
										DirectX::XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f));
}

void Enemy::ShootAtCore() {
	this->timeSinceLastShot += Time::GetInstance().GetDeltaTime();

	if (this->timeSinceLastShot < this->shotCooldown) {
		return;
	}

	Vector3D enemyPosition = this->transform.GetGlobalPosition();
	Vector3D corePosition = this->path[this->maxPathIndex]->transform.GetGlobalPosition();
	Vector3D rayDirection = corePosition - enemyPosition;
	float maxDistance = rayDirection.Length() + 2.f; // Small buffer to ensure hit
	rayDirection.Normalize();

	Ray ray{enemyPosition, rayDirection};
	RayCastData rayCastData = {};

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, maxDistance);
	if (didHit) {
		rayCastData.hitColider.lock()->Interact();
		this->timeSinceLastShot = 0.0f;
	}
}
