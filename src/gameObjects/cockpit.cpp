#include "gameObjects/cockpit.h"
#include "core/physics/sphereCollider.h"
#include "gameObjects/room.h"

void Cockpit::Start() { 
	auto coreCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	coreCollider->SetParent(this->GetPtr());

	coreCollider->SetOnHit([&](float damage) { 
		this->health -= damage;
		if (this->health < 0) {
			// Insert game end call
			Logger::Log("Core Died");
		}
	});
	coreCollider->transform.SetPosition(0, 2, 0);

	//this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	auto tempCockpitMesh = this->factory->CreateStaticGameObject<MeshObject>();
	tempCockpitMesh->SetParent(this->GetPtr());
	tempCockpitMesh->transform.SetPosition(0, 3, 0);
	tempCockpitMesh->transform.SetScale(5, 2.5, 5);
	tempCockpitMesh->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));
	
	this->GameObject3D::Start(); 
}
