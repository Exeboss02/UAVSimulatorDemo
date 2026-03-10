#include "gameObjects/enemies/drone.h"

#include "core/physics/physicsQueue.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "core/physics/vector3D.h"
#include "game/player.h"

Drone::Drone() : Enemy() {
	this->health = Health(100);
	this->movementSpeed = 7.0f;
	this->rotationSpeed = 2.0f;

	this->damage = 5.f;
	this->shootRange = 30.f;
	this->shotCooldown = 1.5f;

	this->playerHitAccuracy = 0.6f;

	this->stuckCheckInterval = 4.f;

	this->transform.SetScale({0.5f, 0.5f, 0.5f});

	this->SetMoveSpeedMode(MoveSpeedMode::NORMAL);
}

void Drone::Start() {
	auto meshObj = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	meshObj->SetParent(this->GetPtr());

	MeshObjData meshData = AssetManager::GetInstance().GetMeshObjData("enemies/NewEnemy.glb:Mesh_0");
	meshObj->SetMesh(meshData);
	meshObj->transform.SetScale({0.3, 0.3, 0.3});
	// meshObj->transform.SetRotationRPY({0, -DirectX::XM_PIDIV2, 0});
	meshObj->transform.SetPosition(0.f, -0.8f, 0.f);
	meshObj->transform.SetRotationRPY({0, 0, 0});

	auto headParent = this->factory->CreateGameObjectOfType<GameObject3D>().lock();
	// headParent->SetParent(this->GetPtr());
	headParent->transform.SetPosition(
		DirectX::XMVectorAdd(this->transform.GetGlobalPosition(), this->headOffsetFromBody));
	this->head = headParent;

	auto head = this->factory->CreateGameObjectOfType<MeshObject>().lock();

	meshData = AssetManager::GetInstance().GetMeshObjData("enemies/NewEnemy.glb:Mesh_1");
	meshData.SetMaterial(3, AssetManager::GetInstance().GetMaterialWeakPtr("eyeMaterial").lock());
	head->SetMesh(meshData);
	head->transform.SetScale({0.15, 0.15, 0.15});
	head->transform.SetPosition({0.0, -0.4, 0.0});
	head->transform.SetRotationRPY({0, -DirectX::XM_PIDIV2, 0});
	head->SetParent(headParent);

	Enemy::Start();
}

void Drone::Tick() { 
	Enemy::Tick();

	float dt = Time::GetInstance().GetDeltaTime();

	if (this->maxPathIndex != 0 && !this->hasFinishedPath) {
		this->MoveAlongPath(dt);

		if (!this->ShootAtPlayer(dt)) {
			this->RotateHead(this->direction, dt);
		}
	} else {
		this->ShootAtCore();
		this->RotateHead(this->direction, dt * 2);
	}
	if (!this->head.expired()) {
		this->head.lock()->transform.SetPosition(
			DirectX::XMVectorAdd(this->transform.GetGlobalPosition(), this->headOffsetFromBody));
	}
}

void Drone::SetMoveSpeedMode(MoveSpeedMode mode) {
	switch (mode) {
	case MoveSpeedMode::NORMAL:
		this->movementSpeed = 2.0f;
		break;
	case MoveSpeedMode::SLOWED:
		this->movementSpeed = 1.0f;
		break;
	case MoveSpeedMode::FAST:
		this->movementSpeed = 3.5f;
		break;
	}
}

bool Drone::ShootAtPlayer(const float deltaTime) { 
	auto player = this->factory->FindObjectOfType<Player>().lock();
	if (!player) {
		Logger::Log("Enemy couldn't find player.");
		return false;
	}
	if (this->head.expired()) {
		Logger::Error("Enemy has no head to shoot from.");
		return false;
	}

	DirectX::XMVECTOR headPosition = this->head.lock()->transform.GetGlobalPosition();
	DirectX::XMVECTOR playerPosition = player->transform.GetGlobalPosition();
	DirectX::XMVECTOR rayDirection = DirectX::XMVectorSubtract(playerPosition, headPosition);
	float rayLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(rayDirection));

	if (rayLength > this->shootRange) {
		return false;
	}

	rayDirection = DirectX::XMVector3Normalize(rayDirection);

	Ray ray{Vector3D(headPosition), Vector3D(rayDirection)};
	RayCastData rayCastData = {};

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, Tag::PLAYER, Tag::ENEMY | Tag::INTERACTABLE,
													  this->shootRange);

	if (!didHit) {
		return false;
	}

	this->RotateHead(rayDirection, deltaTime);

	if (!this->canShoot) {
		return true;
	}

	DirectX::XMVECTOR forward = this->head.lock()->transform.GetGlobalForward();
	if (!DirectX::XMVector4NearEqual(forward, rayDirection, DirectX::XMVectorSet(0.2f, 1.f, 0.2f, 0.2f))) {
		return true;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	float random = dist(gen);
	if (this->playerHitAccuracy < random) {
		Logger::Log("Enemy shooting at player - MISSED!");
		this->canShoot = false;

		// Alter the ray direction to visually indicate a miss
		std::uniform_real_distribution<float> dist2(0.1f, 0.17f);
		float rX = dist2(gen);
		float rY = dist2(gen);
		float rZ = dist2(gen);

		DirectX::XMVECTOR missDir =
			DirectX::XMVectorAdd(rayDirection, DirectX::XMVectorSet(rX, rY, rZ, 0.0f)); // Add some random offset

		this->VisualizeRay(headPosition, missDir, rayCastData.distance);
	} else {
		rayCastData.hitColider.lock()->Hit(this->damage);
		Logger::Log("Enemy shooting at player - HIT!");
		this->canShoot = false;
		this->VisualizeRay(headPosition, rayDirection, rayCastData.distance);

		if (!this->speaker.expired()) {
			SoundClip* shootClip = AssetManager::GetInstance().GetSoundClip("Laser2.wav"); // temporary clip
			this->speaker.lock()->SetRandomPitch(0.9f, 1.1f);
			this->speaker.lock()->Play(shootClip);
		}
	}

	return true;
}
