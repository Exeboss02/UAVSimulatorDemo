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
			this->transform.SetDirection(betweenVec);
			float currentTime = Time::GetInstance().GetSessionTime();
			float delta = currentTime - this->lastFired;
			if (delta > (60 / this->rpm)) {
				this->Fire();
			}
			return;
		}
	} else {
		float currentTime = Time::GetInstance().GetSessionTime();
		float timePast = currentTime - this->lastAttemptedTargeting;
		if (timePast > this->retargetTime) {
			this->SetTargetClosest();
		}
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
	this->retargetTime = Time::GetInstance().GetSessionTime();
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

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, Tag::ENEMY, Tag::PLAYER);
	std::string hitString;
	if (didHit) {

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
