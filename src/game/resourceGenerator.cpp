#include "resourceGenerator.h"

void ResourceGenerator::Start() { 
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));
	
	auto interactCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	interactCollider->SetParent(this->GetPtr());
	interactCollider->SetOnInteract([&](std::shared_ptr<Player> player) { this->Interact(player); });
	
	this->~MeshObject::Start();
}

ResourceGenerator::Interact(std::shared_ptr<Player> player) { 
	float currentTime = Time::GetInstance().GetSessionTime();
	size_t amountGenerated = static_cast<size_t>((currentTime - this->lastGenerated) * this->generatedPerSecond);

	this->lastGenerated = currentTime;

	player->resources.titanium.IncrementAmount(amountGenerated);
}
