#include "gameObjects/turret.h"
#include "core/physics/physicsQueue.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "utilities/time.h"
#include "game/gameManager.h"
#include "gameObjects/rayVis.h"

void Turret::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));
	auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
	collider->transform.SetScale(2, 2, 2);
	collider->SetParent(this->GetPtr());
	collider->SetTag(Tag::PLAYER);
	/*this->shootSound = SoundSourceManager::*/
	auto speaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
	speaker->SetParent(this->GetParent());
	this->speaker = speaker;
	this->shootSound = AssetManager::GetInstance().GetSoundClip("Laser1.wav");
	this->MeshObject::Start();
}

void Turret::Tick() {
	this->MeshObject::Tick();
	// Lowkey illegal considering it is spawned as static
	// Maybe add some is alive check to allow existing gameObjects to be excluded
	if (!this->target.expired()) {
		auto target = this->target.lock();

		DirectX::XMVECTOR betweenVec =
			DirectX::XMVectorSubtract(target->transform.GetGlobalPosition(), this->transform.GetGlobalPosition());

		float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3Dot(betweenVec, betweenVec));

		if (!(distanceSquared > this->range * this->range)) {
			this->SetDirection(betweenVec);
			float currentTime = Time::GetInstance().GetSessionTime();
			float delta = currentTime - this->lastFired;
			if (delta > (60 / this->rpm)) {
				this->Fire();
			}
			return;
		}
	}

	float currentTime = Time::GetInstance().GetSessionTime();
	float timePast = currentTime - this->lastAttemptedTargeting;
	if (timePast > this->retargetTime) {
		this->SetTargetClosest();
		this->lastAttemptedTargeting = Time::GetInstance().GetSessionTime();
	}
	
}

void Turret::SetTarget(std::weak_ptr<GameObject3D> target) { this->target = target; }

std::weak_ptr<GameObject3D> Turret::GetTarget() const { return this->target; }

void Turret::SetTargetClosest() {
	auto& potentialTargets = GameManager::GetInstance()->GetEnemies();
	std::weak_ptr<GameObject3D> currentTarget;

	float currentDistanceSquared = this->range * this->range;

	for (const auto& objectWeak : potentialTargets) {
		const auto& object = objectWeak.lock();
		DirectX::XMVECTOR betweenVec =
			DirectX::XMVectorSubtract(object->transform.GetGlobalPosition(), this->transform.GetGlobalPosition());

		float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3Dot(betweenVec, betweenVec));

		if (distanceSquared < currentDistanceSquared) {
			currentTarget = object;
			currentDistanceSquared = distanceSquared;
		}
	}
	this->SetTarget(currentTarget);
}
void Turret::SetRPM(float rpm) { this->rpm = rpm; }

void Turret::SetRange(float range) { this->range = range; }

void Turret::SetDamage(float damage) { this->damage = damage; }

bool Turret::HasTarget() const { return !this->target.expired(); }

void Turret::Fire() {
	Logger::Log("Boom");
	this->lastFired = Time::GetInstance().GetSessionTime();

	const DirectX::XMVECTOR lookVec = this->transform.GetGlobalForward();
	const DirectX::XMVECTOR posVec = this->transform.GetGlobalPosition();

	Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
	RayCastData rayCastData;

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, Tag::ENEMY, Tag::PLAYER | Tag::INTERACTABLE);
	std::string hitString;
	if (didHit) {

		if (!this->speaker.expired()) {
			this->speaker.lock()->SetRandomPitch(0.8, 1.5);
			this->speaker.lock()->Play(this->shootSound);
		}

		rayCastData.hitColider.lock()->Hit(this->damage);

		// rayVis
		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<RayVis>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->StartDeathTimer(0.05f);
		colliderobj->SetMesh(meshdata);
		colliderobj->GetMesh().SetMaterial(
			0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
		colliderobj->transform.SetPosition(
			DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, rayCastData.distance / 2)));
		colliderobj->transform.SetRotationQuaternion(this->transform.GetGlobalRotation());
		DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance / 2);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		// end of rayVis
	}
}

void Turret::SetDirection(DirectX::XMVECTOR newDirection) {
	DirectX::XMVECTOR desiredDir = DirectX::XMVector3Normalize(newDirection);
	DirectX::XMVECTOR currentDir = DirectX::XMVector3Normalize(this->transform.GetGlobalForward());
	float deltaTime = Time::GetInstance().GetDeltaTime();

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(currentDir, desiredDir));
	dot = std::clamp(dot, -1.0f, 1.0f);

	float angle = acosf(dot);

	float maxStep = this->turnSpeedRPS * deltaTime;

	if (angle > 0.001f) {
		float step = std::min(angle, maxStep);

		DirectX::XMVECTOR axis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(currentDir, desiredDir));

		DirectX::XMMATRIX rot = DirectX::XMMatrixRotationAxis(axis, step);

		DirectX::XMVECTOR newDir = DirectX::XMVector3Normalize(XMVector3TransformNormal(currentDir, rot));

		this->transform.SetDirection(newDir);
	}
}
