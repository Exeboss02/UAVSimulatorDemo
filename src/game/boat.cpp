#include "game/boat.h"
#include "utilities/time.h"
#include "utilities/logger.h"
#include "core/assetManager.h"
#include "core/physics/boxCollider.h"
#include "game/fpvdrone.h"

void Boat::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("Karlskrona/Stridsbat90h.glb:Mesh_0"));
	this->MeshObject::Start();


	auto shipCollider = this->factory->CreateGameObjectOfType<BoxCollider>().lock();
	shipCollider->SetParent(this->GetPtr());
	shipCollider->SetDynamic(true);
	shipCollider->transform.SetScale(2, 1.5, 8);
	shipCollider->ShowDebug(true);


	auto landingCollider = this->factory->CreateGameObjectOfType<BoxCollider>().lock();
	landingCollider->SetParent(this->GetPtr());
	landingCollider->SetDynamic(true);
	landingCollider->SetTag(Tag::INTERACTABLE | Tag::OBJECT);
	landingCollider->ShowDebug(true);
	landingCollider->SetSolid(false);
	landingCollider->SetOnCollision(
		[&](std::weak_ptr<GameObject> object, std::weak_ptr<Collider>) { 
			this->colliding = true; 
			if (auto drone = dynamic_pointer_cast<FPVDrone>(object.lock())) {
				drone->targetColliding = true;
				drone->SetText(std::to_string(this->time));
			}
		});
	landingCollider->transform.SetPosition(0, 3.5, -5);
	landingCollider->transform.SetScale(2, 2, 3);
}

void Boat::Tick() { 
	if (colliding) {
		this->time -= Time::GetInstance().GetDeltaTime();
		
	} else {

		this->time = landingTime;
	}
	float rotation = this->speed* Time::GetInstance().GetSessionTime() / this -> radius;

	float z = sin(rotation) * this->radius;
	float x = cos(rotation) * this->radius;

	float forward_z = sin(rotation + DirectX::XM_PIDIV2);
	float forward_x = cos(rotation + DirectX::XM_PIDIV2);

	this->transform.SetDirection(DirectX::XMVectorSet(forward_x, 0, forward_z, 0));

	this->transform.SetPosition(this->rotationPoint.x + x, this->rotationPoint.y, this->rotationPoint.z + z);

	this->MeshObject::Tick();
}

void Boat::PhysicsTick() { 
	this->MeshObject::PhysicsTick();
	this->colliding = false;
}

void Boat::SetRotationPoint(DirectX::XMFLOAT3 point) { 
	this->rotationPoint = point; 
}

void Boat::SetRadius(float radius) { this->radius = radius; }

float Boat::GetRadius() const { return this->radius; }

DirectX::XMFLOAT3 Boat::GetRotationPoint() const { return this->rotationPoint; }
