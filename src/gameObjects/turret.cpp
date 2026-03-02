#include "gameObjects/turret.h"
#include "core/physics/physicsQueue.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "utilities/time.h"
#include "game/gameManager.h"
#include "gameObjects/rayVis.h"

void Turret::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("Buildings/Turret.glb:Mesh_0"));
	auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
	collider->transform.SetScale(2, 2, 2);
	collider->transform.SetPosition(0, 0.5, 0);
	collider->SetParent(this->GetPtr());
	collider->SetTag(Tag::PLAYER);
	auto colliderChild = collider.Get();
	/*this->shootSound = SoundSourceManager::*/
	auto speaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
	speaker->SetParent(this->GetParent());
	this->speaker = speaker;
	this->shootSound = AssetManager::GetInstance().GetSoundClip("Laser1.wav");

	SoundClip* clip = AssetManager::GetInstance().GetSoundClip("Build2.wav");
	this->speaker.lock()->transform.SetPosition(this->transform.GetPosition()); //global position came from 0, 0, 0?
	this->speaker.lock()->SetRandomPitch(0.8f, 1.0f);
	this->speaker.lock()->SetGain(1.0f);
	this->speaker.lock()->Play(clip);

	this->MeshObject::Start();

	//collider.Init();

	//colliderChild->ShowDebug(true);

	auto frame = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	frame->SetMesh(AssetManager::GetInstance().GetMeshObjData("Buildings/Turret.glb:Mesh_2"));
	frame->SetParent(this->GetPtr());
	this->framePart = frame;

	auto turretParent = this->factory->CreateGameObjectOfType<GameObject3D>().lock();
	turretParent->transform.SetPosition(0, 1.15, 0);
	turretParent->SetParent(this->GetPtr());
	this->turretPart = turretParent;

	auto turret = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	turret->SetMesh(AssetManager::GetInstance().GetMeshObjData("Buildings/Turret.glb:Mesh_1"));
	turret->transform.SetRotationRPY(0, 0, DirectX::XM_PI);
	turret->SetParent(turretParent->GetPtr());
}

void Turret::Tick() {
	this->MeshObject::Tick();
	// Lowkey illegal considering it is spawned as static
	// Maybe add some is alive check to allow existing gameObjects to be excluded
	if (!this->target.expired()) {
		auto target = this->target.lock();
		auto turret = this->turretPart.lock();

		DirectX::XMVECTOR betweenVec =
			DirectX::XMVectorSubtract(target->transform.GetGlobalPosition(), turret->transform.GetGlobalPosition());

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

	auto turret = this->turretPart.lock();

	float currentDistanceSquared = this->range * this->range;

	for (const auto& objectWeak : potentialTargets) {
		const auto& object = objectWeak.lock();
		DirectX::XMVECTOR betweenVec =
			DirectX::XMVectorSubtract(object->transform.GetGlobalPosition(), turret->transform.GetGlobalPosition());

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

	auto turret = this->turretPart.lock();

	const DirectX::XMVECTOR lookVec = turret->transform.GetGlobalForward();
	const DirectX::XMVECTOR posVec = turret->transform.GetGlobalPosition();

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
			0, AssetManager::GetInstance().GetMaterialWeakPtr("blueUnlit").lock());
		colliderobj->SetCastShadow(false);
		colliderobj->transform.SetPosition(
			DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, rayCastData.distance / 2)));
		colliderobj->transform.SetRotationQuaternion(turret->transform.GetGlobalRotation());
		DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance / 2);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		// end of rayVis
	}
}

void Turret::SetDirection(DirectX::XMVECTOR newDirection) {
	if (this->turretPart.expired()) {
		Logger::Warn("No turret");
		return;
	}
	if (this->framePart.expired()) {
		Logger::Warn("No frame");
		return;
	}

	auto turret = this->turretPart.lock();

	DirectX::XMVECTOR desiredDir = DirectX::XMVector3Normalize(newDirection);
	DirectX::XMVECTOR currentDir = DirectX::XMVector3Normalize(turret->transform.GetGlobalForward());
	float deltaTime = Time::GetInstance().GetDeltaTime();

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(currentDir, desiredDir));
	dot = std::clamp(dot, -1.0f, 1.0f);

	float angle = acosf(dot);

	float maxStep = this->turnSpeedRPS * deltaTime;

	DirectX::XMVECTOR newDir;

	// If already close enough, snap to desired
	if (angle <= maxStep) {
		newDir = desiredDir;
	} else {
		float step = std::min(angle, maxStep);

		DirectX::XMVECTOR axis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(currentDir, desiredDir));

		if (DirectX::XMVector3LengthSq(axis).m128_f32[0] < 0.0001f) {
			//axis = DirectX::XMVectorSet(0, 1, 0, 0); // fallback axis
			axis = DirectX::XMVector3Orthogonal(currentDir);
		}

		DirectX::XMVECTOR rot = DirectX::XMQuaternionRotationAxis(axis, step);

		newDir = DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(currentDir, rot));
	}

	turret->transform.SetDirection(newDir);

	DirectX::XMVECTOR frameDir{newDir.m128_f32[0], 0, newDir.m128_f32[2]};
	frameDir = DirectX::XMVector3Normalize(frameDir);

	auto frame = this->framePart.lock();
	frame->transform.SetDirection(frameDir);
}
