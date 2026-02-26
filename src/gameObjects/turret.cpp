#include "gameObjects/turret.h"
#include "core/physics/physicsQueue.h"
#include "core/physics/ray.h"
#include "core/physics/rayCaster.h"
#include "utilities/time.h"

void Turret::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));
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
	this->target = std::weak_ptr<GameObject3D>();
}

void Turret::SetTarget(std::weak_ptr<GameObject3D> target) { this->target = target; }

std::weak_ptr<GameObject3D> Turret::GetTarget() const { return this->target; }

void Turret::SetTargetClosest(const std::vector<std::weak_ptr<GameObject3D>> potentialTargets) {
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

	const DirectX::XMVECTOR lookVec = DirectX::XMVector3Normalize(this->transform.GetDirectionVector());
	const DirectX::XMVECTOR posVec = this->transform.GetPosition();

	Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
	RayCastData rayCastData;
	Logger::Log("shooting ray");

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData);
	std::string hitString;
	if (didHit) {

		auto collider = rayCastData.hitColider.lock();
		collider->Hit(this->damage);

		hitString = "hit";

		// rayVis
		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->SetMesh(meshdata);
		colliderobj->GetMesh().SetMaterial(
			0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
		colliderobj->transform.SetPosition(
			DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, rayCastData.distance / 2)));
		colliderobj->transform.SetRotationQuaternion(this->transform.GetGlobalRotation());
		DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance / 2);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		// end of rayVis
	} else {
		hitString = "miss";
	}

	Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
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
