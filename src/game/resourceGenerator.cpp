#include "game/resourceGenerator.h"
#include "UI/canvasObject.h"
#include "UI/interactionPrompt.h"
#include "UI/text.h"
#include "core/window.h"
#include "gameObjects/cameraObject.h"
#include "rendering/renderQueue.h"

// d3d11
#include <DirectXMath.h>

// std
#include <string>

void ResourceGenerator::Start() {
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	auto interactCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	interactCollider->SetParent(this->GetPtr());
	// mark this collider as interactable so player raycasts can filter hits
	interactCollider->SetTag(Tag::INTERACTABLE);
	interactCollider->SetOnInteract([&](std::shared_ptr<Player> player) { this->Interact(player); });
	interactCollider->SetOnHover([&] { this->Hover(); });
	interactCollider->transform.SetScale(2, 2, 2);

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

void ResourceGenerator::Hover() {
	try {
		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (promptWeak.expired()) return;
		auto prompt = promptWeak.lock();
		if (!prompt) return;

		std::string txt = "Press \"F\" to collect resources";
		DirectX::XMVECTOR worldPos = this->transform.GetGlobalPosition();
		prompt->Show(txt, worldPos);
	} catch (const std::exception& e) {
		Logger::Error("ResourceGenerator::Hover exception: ", e.what());
	} catch (...) {
		Logger::Error("ResourceGenerator::Hover unknown exception");
	}
}
