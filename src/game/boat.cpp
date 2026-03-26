#include "game/boat.h"
#include "utilities/time.h"
#include "utilities/logger.h"
#include "core/assetManager.h"
#include "core/physics/boxCollider.h"

void Boat::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("Karlskrona/Stridsbat90h.glb:Mesh_0"));
	this->MeshObject::Start();

	auto landingCollider = this->factory->CreateGameObjectOfType<BoxCollider>().lock();
	landingCollider->SetParent(this->GetPtr());
	landingCollider->SetDynamic(true);
	landingCollider->ShowDebug(true);

	landingCollider->transform.SetPosition(0, 3, -5);
	landingCollider->transform.SetScale(2, 2, 3);
}

void Boat::Tick() { 

	float rotation = this->speed* Time::GetInstance().GetSessionTime() / this -> radius;

	float z = sin(rotation) * this->radius;
	float x = cos(rotation) * this->radius;

	float forward_z = sin(rotation + DirectX::XM_PIDIV2);
	float forward_x = cos(rotation + DirectX::XM_PIDIV2);

	this->transform.SetDirection(DirectX::XMVectorSet(forward_x, 0, forward_z, 0));

	this->transform.SetPosition(this->rotationPoint.x + x, this->rotationPoint.y, this->rotationPoint.z + z);

	this->MeshObject::Tick();
}

void Boat::SetRotationPoint(DirectX::XMFLOAT3 point) { 
	this->rotationPoint = point; 
}

void Boat::SetRadius(float radius) { this->radius = radius; }

float Boat::GetRadius() const { return this->radius; }

DirectX::XMFLOAT3 Boat::GetRotationPoint() const { return this->rotationPoint; }
