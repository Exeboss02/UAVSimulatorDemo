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
	float amountGenerated = (currentTime - this->lastGenerated) * this->generatedPerSecond + this->change;

	size_t amountGained = static_cast<size_t>(amountGenerated);

	this->change = amountGenerated - amountGained;

	this->lastGenerated = currentTime;

	player->resources.GetResource(this->resourceType).IncrementAmount(amountGenerated);

	// hide interaction prompt for a short time so text disappears when interacting
	try {
		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (!promptWeak.expired()) {
			auto prompt = promptWeak.lock();
			if (prompt) prompt->Hide();
		}
	} catch (const std::exception& e) {
		Logger::Error("ResourceGenerator::Interact Hide prompt exception: ", e.what());
	} catch (...) {
		Logger::Error("ResourceGenerator::Interact Hide prompt unknown exception");
	}

	// disable showing the prompt for 1 second
	this->hoverDisabledUntil = currentTime + 0.5f;
}

void ResourceGenerator::Hover() {
	try {
		float currentTime = Time::GetInstance().GetSessionTime();
		if (currentTime < this->hoverDisabledUntil) return;
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
