#include "game/boat.h"
#include "utilities/time.h"
#include "core/assetManager.h"

void Boat::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("Karlskrona/cb-90_fast_class_assault_craft.glb:Mesh_0"));
	this->MeshObject::Start();
}

void Boat::Tick() { 

	float rotation = this->speed * Time::GetInstance().GetSessionTime() / (this->radius * DirectX::XM_2PI);

	float z = sin(rotation);
	float x = cos(rotation);

	float forward_z = sin(Time::GetInstance().GetSessionTime() / (this->radius * DirectX::XM_2PI));
	float forward_x = cos(Time::GetInstance().GetSessionTime() / (this->radius * DirectX::XM_2PI));

	this->transform.SetPosition(this->rotationPoint.x + x, this->rotationPoint.y, this->rotationPoint.z + z);
}

void Boat::SetRotationPoint(DirectX::XMFLOAT3 point) { 
	this->rotationPoint = point; }

void Boat::SetRadius(float radius) { this->radius = radius; }

float Boat::GetRadius() const { return this->radius; }

DirectX::XMFLOAT3 Boat::GetRotationPoint() const { return this->rotationPoint; }
