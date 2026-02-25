#include "game/resourceGenerator.h"

void ResourceGenerator::Start() {
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	auto interactCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	interactCollider->SetParent(this->GetPtr());
	interactCollider->SetOnInteract([&](std::shared_ptr<Player> player) { this->Interact(player); });
	interactCollider->transform.SetScale(2, 2, 2);

	this->lastGenerated = Time::GetInstance().GetSessionTime();

	this->MeshObject::Start();
}

void ResourceGenerator::SetGenerationSpeed(float generatedPerSecond) { this->generatedPerSecond = generatedPerSecond; }

size_t ResourceGenerator::GetCurrentlyGenerated() const {
	float currentTime = Time::GetInstance().GetSessionTime();
	size_t amountGenerated = static_cast<size_t>((currentTime - this->lastGenerated) * this->generatedPerSecond);
	return amountGenerated;
}

ResourceType ResourceGenerator::GetResourceType() const { return this->resourceType; }

void ResourceGenerator::SetResourceType(ResourceType type) { this->resourceType = type; }

void ResourceGenerator::Interact(std::shared_ptr<Player> player) {
	float currentTime = Time::GetInstance().GetSessionTime();
	size_t amountGenerated = static_cast<size_t>((currentTime - this->lastGenerated) * this->generatedPerSecond);

	this->lastGenerated = currentTime;

	player->resources.GetResource(this->resourceType).IncrementAmount(amountGenerated);

	Logger::Log("Player has ", player->resources.titanium.GetAmount(), " of currently generated resource");
}
