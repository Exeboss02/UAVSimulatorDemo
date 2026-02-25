#include "gameObjects/enemy.h"

#include "core/physics/vector3D.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "core/physics/physicsQueue.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/meshObjData.h"
#include "game/player.h"
#include "core/assetManager.h"

#include "utilities/logger.h"

Enemy::Enemy() : GameObject3D(), health(100), path({}), maxPathIndex(0), currentPathIndex(0),
	hasFinishedPath(false), canShoot(true), shotCooldown(1.5f), timeSinceLastShot(0.0f) {
	this->direction = DirectX::XMVectorSet(0, 0, 1, 0);
	this->targetRotation = DirectX::XMQuaternionIdentity();
	this->SetMoveSpeedMode(MoveSpeedMode::NORMAL);
	this->transform.SetScale({0.5f, 0.5f, 0.5f});
}

void Enemy::Start() { 
	auto meshObj = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	meshObj->SetParent(this->GetPtr());

	MeshObjData meshData = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"); // TODO: Add enemy mesh
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
	this->UpdateShootCooldown();

	if (this->maxPathIndex != 0 && !this->hasFinishedPath) {
		this->MoveAlongPath();
		this->ShootAtPlayer();
	}
	else {
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
	if (this->currentPathIndex >= this->maxPathIndex) { // Stop on node before core.
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

		this->targetRotation = DirectX::XMQuaternionRotationMatrix(rotationMatrix);
	}

	float dt = Time::GetInstance().GetDeltaTime();

	// Smoothly lerp rotation
	DirectX::XMVECTOR currentRotation = this->transform.GetRotationQuaternion();
	float deltaRotation = this->rotationSpeed * dt;
	DirectX::XMVECTOR newRotation = DirectX::XMQuaternionSlerp(currentRotation, this->targetRotation, deltaRotation);
	this->transform.SetRotationQuaternion(newRotation);

	this->transform.Move(this->direction, this->movementSpeed * dt);
}

bool Enemy::IsAtCurrentPathNode() { 
	return DirectX::XMVector4NearEqual(this->transform.GetGlobalPosition(),
										this->path[this->currentPathIndex]->transform.GetGlobalPosition(),
										DirectX::XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f));
}

void Enemy::UpdateShootCooldown() {
	if (!this->canShoot) {
		float deltaTime = Time::GetInstance().GetDeltaTime();
		this->timeSinceLastShot += deltaTime;

		if (this->timeSinceLastShot >= this->shotCooldown) {
			this->canShoot = true;
			this->timeSinceLastShot = 0.0f;
		}
	}
}

void Enemy::ShootAtCore() {
	if (!this->canShoot) return;

	Vector3D enemyPosition = this->transform.GetGlobalPosition();
	Vector3D corePosition = this->path[this->maxPathIndex]->transform.GetGlobalPosition();
	Vector3D rayDirection = corePosition - enemyPosition;
	float maxDistance = rayDirection.Length() + 1.f; // Small bias
	rayDirection.Normalize();

	Vector3D adjustedPos = enemyPosition + rayDirection * 1.f;
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(enemyPosition.GetX(), enemyPosition.GetY(), enemyPosition.GetZ(), 1.0f);
	DirectX::XMVECTOR dir = DirectX::XMVectorSet(rayDirection.GetX(), rayDirection.GetY(), rayDirection.GetZ(), 0.0f);
	Ray ray{adjustedPos, rayDirection};
	RayCastData rayCastData = {};

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, maxDistance);
	if (didHit && !rayCastData.hitColider.expired()) {
		auto hitCollider = rayCastData.hitColider.lock();
		if (hitCollider) {
			hitCollider->Hit(this->damage);
			Logger::Log("Enemy shooting at core");
			this->canShoot = false;

					MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
					auto colliderobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();
					auto colliderobj = colliderobjWeak.lock();
					colliderobj->SetMesh(meshdata);
					colliderobj->GetMesh().SetMaterial(0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
					DirectX::XMVECTOR direction = DirectX::XMVectorSet(rayDirection.GetX(), rayDirection.GetY(), rayDirection.GetZ(), 0.0f);
					colliderobj->transform.SetPosition(DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(direction, rayCastData.distance / 2)));
					colliderobj->transform.SetRotationQuaternion(dir);
					DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance);
					colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
				}
			}
		}

void Enemy::ShootAtPlayer() { 
	if (!this->canShoot) return;

	auto player = this->factory->FindObjectOfType<Player>().lock();
	if (!player) {
		Logger::Log("Enemy couldn't find player.");
		return;
	}

	Vector3D enemyPosition = this->transform.GetGlobalPosition();
	Vector3D playerPosition = player->transform.GetGlobalPosition();
	Vector3D rayDirection = playerPosition - enemyPosition;
	float maxDistance = rayDirection.Length() + 1.f; // Small bias;
	rayDirection.Normalize();

	Vector3D adjustedPos = enemyPosition + rayDirection * 1.f; // Move the ray origin slightly forward to avoid hitting self
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(enemyPosition.GetX(), enemyPosition.GetY(), enemyPosition.GetZ(), 1.0f);
	DirectX::XMVECTOR dir = DirectX::XMVectorSet(rayDirection.GetX(), rayDirection.GetY(), rayDirection.GetZ(), 0.0f);
	Ray ray{adjustedPos, rayDirection};
	RayCastData rayCastData = {};

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, maxDistance);
	if (didHit) {
		if (auto hitCollider = rayCastData.hitColider.lock()) {
			if (hitCollider->GetParent().lock().get() == player.get()) {
				rayCastData.hitColider.lock()->Hit(this->damage);
				Logger::Log("Enemy shooting at player");
				this->canShoot = false;

				MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
				auto colliderobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();
				auto colliderobj = colliderobjWeak.lock();
				colliderobj->SetMesh(meshdata);
				colliderobj->GetMesh().SetMaterial(
					0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
				DirectX::XMVECTOR direction =
					DirectX::XMVectorSet(rayDirection.GetX(), rayDirection.GetY(), rayDirection.GetZ(), 0.0f);
				colliderobj->transform.SetPosition(
					DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(direction, rayCastData.distance / 2)));
				colliderobj->transform.SetRotationQuaternion(dir);
				DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance);
				colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
			}
		}
	}
}
