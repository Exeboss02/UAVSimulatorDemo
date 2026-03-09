#include "gameObjects/enemy.h"

#include "core/assetManager.h"
#include "core/physics/physicsQueue.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "core/physics/vector3D.h"
#include "game/player.h"
#include "gameObjects/meshObjData.h"
#include "gameObjects/meshObject.h"

#include "utilities/logger.h"

#include "gameObjects/rayVis.h"

#include <random>

Enemy::Enemy()
	: GameObject3D(), health(100), path({}), maxPathIndex(0), currentPathIndex(0), hasFinishedPath(false),
	  canShoot(true), shotCooldown(1.5f), timeSinceLastShot(0.0f), isSlowed(false), slowDuration(0.0f), timeSinceSlowed(0.0f), 
		playerHitAccuracy(0.6f), timeStuck(0.f), stuckCheckInterval(4.f) {
	this->direction = DirectX::XMVectorSet(0, 0, 1, 0);
	this->bodyTargetRotation = DirectX::XMQuaternionIdentity();
	this->SetMoveSpeedMode(MoveSpeedMode::NORMAL);
	this->transform.SetScale({0.5f, 0.5f, 0.5f});
}

void Enemy::Start() {
	auto meshObj = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	meshObj->SetParent(this->GetPtr());

	MeshObjData meshData =
		AssetManager::GetInstance().GetMeshObjData("enemies/NewEnemy.glb:Mesh_0");
	meshObj->SetMesh(meshData);
	meshObj->transform.SetScale({0.3, 0.3, 0.3});
	//meshObj->transform.SetRotationRPY({0, -DirectX::XM_PIDIV2, 0});
	meshObj->transform.SetPosition(0.f, -0.8f, 0.f);
	meshObj->transform.SetRotationRPY({0, 0, 0});

	auto headParent = this->factory->CreateGameObjectOfType<GameObject3D>().lock();
	//headParent->SetParent(this->GetPtr());
	headParent->transform.SetPosition(
		DirectX::XMVectorAdd(this->transform.GetGlobalPosition(), this->headOffsetFromBody));
	this->head = headParent;

	auto head = this->factory->CreateGameObjectOfType<MeshObject>().lock();

	meshData = AssetManager::GetInstance().GetMeshObjData("enemies/NewEnemy.glb:Mesh_1");
	head->SetMesh(meshData);
	head->transform.SetScale({0.15, 0.15, 0.15});
	head->transform.SetPosition({0.0, -0.4, 0.0});
	head->transform.SetRotationRPY({0, -DirectX::XM_PIDIV2, 0});
	head->SetParent(headParent);

	auto collider = this->factory->CreateGameObjectOfType<SphereCollider>().lock();
	collider->transform.SetScale({2, 2, 2});
	collider->SetParent(this->GetPtr());
	collider->SetTag(Tag::ENEMY);
	collider->SetDynamic(true);
	//collider->ShowDebug(true);

	//Create speaker object
    this->speaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
    this->speaker.lock()->SetParent(this->GetPtr());
	this->speaker.lock()->SetGain(0.85f);
	

	collider->SetOnHit([&](float damage) {
		this->health.Decrement(damage);
		if (this->health.Get() <= 0) {

			//temporary speaker that deletes itself when finnished playing, since this object will get deleted now
			SoundClip* deathClip = AssetManager::GetInstance().GetSoundClip("RobotBreach.wav");
			std::weak_ptr<SoundSourceObject> deathSpeaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
			deathSpeaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
			deathSpeaker.lock()->SetDeleteWhenFinnished(true);
			deathSpeaker.lock()->SetGain(1.0f);
			deathSpeaker.lock()->Play(deathClip);

			this->KillSelf();
		}
			
		int randomInt = RandomInt(1, 3);
		std::string soundName = "RobotHit" + std::to_string(randomInt) + ".wav";
		SoundClip* clip = AssetManager::GetInstance().GetSoundClip(soundName);
	
		if(!this->speaker.expired()) this->speaker.lock()->Play(clip);
	});
	this->hitbox = collider;
}

void Enemy::Tick() {
	float dt = Time::GetInstance().GetDeltaTime();
	
	this->UpdateShootCooldown(dt);
	this->UpdateSlowEffect(dt);
	this->IsStuckOnPath(dt);
	
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

void Enemy::SlowDownEnemy(float durationInSec) {
	this->SetMoveSpeedMode(MoveSpeedMode::SLOWED);
	this->isSlowed = true;
	this->slowDuration = durationInSec;
	this->timeSinceSlowed = 0.0f;
}

void Enemy::UpdateSlowEffect(const float deltaTime) {
	if (this->isSlowed) {
		this->timeSinceSlowed += deltaTime;

		if (this->timeSinceSlowed >= this->slowDuration) {
			this->SetMoveSpeedMode(MoveSpeedMode::NORMAL);
			this->isSlowed = false;
			this->timeSinceSlowed = 0.0f;
		}
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

	// Calculate initial direction from first path node to second
	if (this->maxPathIndex > 0) {
		this->direction = DirectX::XMVectorSubtract(this->path[1]->transform.GetGlobalPosition(),
													this->path[0]->transform.GetGlobalPosition());
		this->direction = DirectX::XMVector3Normalize(this->direction);

		// Set head direction to match initial path direction
		if (!this->head.expired()) {
			this->head.lock()->transform.SetDirection(this->direction);
		}
	}
}

void Enemy::KillSelf() { 
	this->factory->QueueDeleteGameObject(this->GetPtr()); 
	this->factory->QueueDeleteGameObject(this->head);
}

void Enemy::DecrementHealth(size_t amount) {
	this->health.Decrement(static_cast<int>(amount));
	if (this->health.IsDead()) {
		this->KillSelf();
	}
}

void Enemy::IncrementHealth(size_t amount) { this->health.Increment(static_cast<int>(amount)); }


void Enemy::MoveAlongPath(const float deltaTime) {
	if (this->currentPathIndex >= this->maxPathIndex) { // Stop on node before core.
		this->hasFinishedPath = true;
		return;
	}

	if (this->IsAtCurrentPathNode()) {
		this->currentPathIndex++;
		this->timeStuck = 0.f;
		
		this->CalulateDirectionToTarget();
		this->CalulateTargetRotation();
	}

	this->RotateBody(deltaTime);

	if (auto player = this->factory->FindObjectOfType<Player>().lock()) {
		DirectX::XMVECTOR toPlayer = DirectX::XMVectorSubtract(player->transform.GetGlobalPosition(), this->transform.GetGlobalPosition());
		
		//float distanceToPlayer = DirectX::XMVectorGetX(DirectX::XMVector4Length(toPlayer));
		//if (distanceToPlayer < this->shootRange) {
		//	toPlayer = DirectX::XMVector3Normalize(toPlayer);
		//	this->RotateHead(toPlayer, deltaTime);
		//} 
		//else { // If player is out of range, rotate head towards movement direction
		//	this->RotateHead(this->direction, deltaTime);
		//}
	}

	if (this->head.expired()) {
		Logger::Log("Enemy has no head to move.");
		return;
	}
	this->transform.Move(this->direction, this->movementSpeed * deltaTime);
}

bool Enemy::IsAtCurrentPathNode() {
	return DirectX::XMVector4NearEqual(this->transform.GetGlobalPosition(),
									   this->path[this->currentPathIndex]->transform.GetGlobalPosition(),
									   DirectX::XMVectorSet(0.7f, 0.7f, 0.7f, 0.7f));
}

void Enemy::CalulateDirectionToTarget() {
	this->direction = DirectX::XMVectorSubtract(this->path[this->currentPathIndex]->transform.GetGlobalPosition(),
												this->transform.GetGlobalPosition());
	this->direction = DirectX::XMVector3Normalize(this->direction);
}

void Enemy::CalulateTargetRotation() {
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, this->direction);
	right = DirectX::XMVector3Normalize(right);
	DirectX::XMVECTOR actualUp = DirectX::XMVector3Cross(this->direction, right);

	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();
	rotationMatrix.r[0] = right;
	rotationMatrix.r[1] = actualUp;
	rotationMatrix.r[2] = this->direction;
	rotationMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	//this->bodyTargetRotation = DirectX::XMQuaternionRotationMatrix(rotationMatrix);

	DirectX::XMVECTOR targetRotation = DirectX::XMQuaternionRotationMatrix(rotationMatrix);

	// Apply compensating rotation for the mesh's initial +90° Y rotation
	DirectX::XMVECTOR meshOffsetRotation =
		DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
										  -DirectX::XM_PIDIV2 // -90 degrees to compensate for mesh's +90
		);

	this->bodyTargetRotation = DirectX::XMQuaternionMultiply(meshOffsetRotation, targetRotation);
}

void Enemy::IsStuckOnPath(const float deltaTime) {
	this->timeStuck += deltaTime;

	if (this->timeStuck >= this->stuckCheckInterval) {

		this->CalulateDirectionToTarget();
		this->CalulateTargetRotation();
		this->timeStuck = 0.f;
	}
}

void Enemy::UpdateShootCooldown(const float deltaTime) {
	if (!this->canShoot) {
		this->timeSinceLastShot += deltaTime;

		if (this->timeSinceLastShot >= this->shotCooldown) {
			this->canShoot = true;
			this->timeSinceLastShot = 0.0f;
		}
	}
}

void Enemy::ShootAtCore() {
	if (!this->canShoot) return;

	if (this->path.empty()) {
		Logger::Log("Enemy cannot shoot at core: path is empty");
		return;
	}

	if (this->maxPathIndex >= this->path.size()) {
		Logger::Log("Enemy cannot shoot at core: maxPathIndex out of bounds");
		return;
	}

	if(!this->speaker.expired())
	{
		SoundClip* shootClip = AssetManager::GetInstance().GetSoundClip("Laser2.wav"); //temporary clip
		this->speaker.lock()->SetRandomPitch(0.9f, 1.1f);
		this->speaker.lock()->Play(shootClip);
	}

	Vector3D enemyPosition = this->transform.GetGlobalPosition();
	Vector3D corePosition = this->path[this->maxPathIndex]->transform.GetGlobalPosition();
	Vector3D rayDirection = corePosition - enemyPosition;
	float rayLength = rayDirection.Length();
	rayDirection.Normalize();

	// Offset ray origin to avoid hitting self
	Vector3D adjustedPos = enemyPosition + rayDirection * 1.5f;
	float maxDistance = rayLength + 1.f; // Subtract offset, add small bias

	DirectX::XMVECTOR pos = DirectX::XMVectorSet(adjustedPos.GetX(), adjustedPos.GetY(), adjustedPos.GetZ(), 1.0f);
	DirectX::XMVECTOR dir = DirectX::XMVectorSet(rayDirection.GetX(), rayDirection.GetY(), rayDirection.GetZ(), 0.0f);
	Ray ray{adjustedPos, rayDirection};
	RayCastData rayCastData = {};

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, maxDistance);

	// Always trigger cooldown after attempting to shoot
	this->canShoot = false;

	if (didHit && !rayCastData.hitColider.expired()) {
		auto hitCollider = rayCastData.hitColider.lock();
		if (hitCollider) {
			Logger::Log("Enemy shooting at core - HIT!");
			hitCollider->Hit(this->damage);
			this->VisualizeRay(pos, dir, rayCastData.distance);
		}
	}
}

bool Enemy::ShootAtPlayer(const float deltaTime) {
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
													  Tag::NOIGNORE, this->shootRange);

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

		DirectX::XMVECTOR missDir = DirectX::XMVectorAdd(
			rayDirection, DirectX::XMVectorSet(rX, rY, rZ, 0.0f)); // Add some random offset

		this->VisualizeRay(headPosition, missDir, rayCastData.distance);
	} 
	else {
		rayCastData.hitColider.lock()->Hit(this->damage);
		Logger::Log("Enemy shooting at player - HIT!");
		this->canShoot = false;
		this->VisualizeRay(headPosition, rayDirection, rayCastData.distance);

		if(!this->speaker.expired()) {
			SoundClip* shootClip = AssetManager::GetInstance().GetSoundClip("Laser2.wav"); //temporary clip
			this->speaker.lock()->SetRandomPitch(0.9f, 1.1f);
			this->speaker.lock()->Play(shootClip);
		}
	}

	return true;
}

void Enemy::RotateBody(const float deltaTime) {
	// Smoothly lerp rotation
	DirectX::XMVECTOR currentRotation = this->transform.GetRotationQuaternion();
	float deltaRotation = this->rotationSpeed * deltaTime;
	DirectX::XMVECTOR newRotation = DirectX::XMQuaternionSlerp(currentRotation, this->bodyTargetRotation, deltaRotation);
	this->transform.SetRotationQuaternion(newRotation); // Make sure this is body's transform, not head's
}

void Enemy::RotateHead(DirectX::XMVECTOR& newDirection, const float deltaTime) {
	if (this->head.expired()) {
		Logger::Error("Enemy has no head to rotate.");
		return;
	}

	// Check if we are already facing the desired direction
	if (DirectX::XMVector4NearEqual(
			this->head.lock()->transform.GetGlobalForward(), // Make sure this is head's forward, not body's
			newDirection, DirectX::XMVectorSet(0.01f, 0.01f, 0.01f, 0.01f))) {
		return;
	}

	DirectX::XMVECTOR currentDir = DirectX::XMVector3Normalize(this->head.lock()->transform.GetGlobalForward());
	DirectX::XMVECTOR desiredDir = DirectX::XMVector3Normalize(newDirection);

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(currentDir, newDirection));
	dot = std::clamp(dot, -1.0f, 1.0f);

	float maxStep = this->rotationSpeed * 0.5f * deltaTime;

	DirectX::XMVECTOR newDir;

	float angle = acosf(dot);
	if (angle <= maxStep) {
		newDir = desiredDir;
	} else {
		float step = std::min(angle, maxStep);
		DirectX::XMVECTOR axis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		// Determine rotation direction (clockwise or counter-clockwise)
		DirectX::XMVECTOR cross = DirectX::XMVector3Cross(currentDir, desiredDir);
		if (DirectX::XMVectorGetY(cross) < 0.0f) {
			step = -step;
		}

		if (DirectX::XMVector3LengthSq(axis).m128_f32[0] < 0.0001f) {
			// axis = DirectX::XMVectorSet(0, 1, 0, 0); // fallback axis
			axis = DirectX::XMVector3Orthogonal(currentDir);
		}

		DirectX::XMVECTOR rot = DirectX::XMQuaternionRotationAxis(axis, step);

		newDir = DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(currentDir, rot));
	}

	this->head.lock()->transform.SetDirection(newDir);
}

void Enemy::VisualizeRay(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction, float distance) {
	MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
	auto colliderobjWeak = this->factory->CreateGameObjectOfType<RayVis>();
	auto colliderobj = colliderobjWeak.lock();
	colliderobj->StartDeathTimer(1);
	colliderobj->SetMesh(meshdata);
	colliderobj->GetMesh().SetMaterial(0,
									   AssetManager::GetInstance().GetMaterialWeakPtr("enemyLaser").lock());
	colliderobj->SetCastShadow(false);

	colliderobj->transform.SetPosition(DirectX::XMVectorAdd(position, DirectX::XMVectorScale(direction, distance / 2)));

	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, direction);
	right = DirectX::XMVector3Normalize(right);
	DirectX::XMVECTOR actualUp = DirectX::XMVector3Cross(direction, right);

	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();
	rotationMatrix.r[0] = right;
	rotationMatrix.r[1] = actualUp;
	rotationMatrix.r[2] = direction;
	rotationMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	DirectX::XMVECTOR rayRotation = DirectX::XMQuaternionRotationMatrix(rotationMatrix);
	colliderobj->transform.SetRotationQuaternion(rayRotation);

	DirectX::XMFLOAT3 scale(0.01f, 0.01f, distance);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
}
