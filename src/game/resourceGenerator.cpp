#include "game/resourceGenerator.h"
#include "UI/canvasObject.h"
#include "UI/interactionPrompt.h"
#include "UI/text.h"
#include "core/input/inputManager.h"
#include "core/window.h"
#include "game/gameManager.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/room.h"
#include "rendering/renderQueue.h"

// d3d11
#include <DirectXMath.h>

// std
#include <string>

void ResourceGenerator::Start() {
	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("meshes/generator.glb:Mesh_0"));
	this->transform.SetScale(0.5f, 1.2f, 0.5f);
	this->transform.SetPosition(0, 1.8f, 0);

	this->titanium.SetGenerationSpeed(60);
	this->carbonFiber.SetGenerationSpeed(30);
	this->lubricant.SetGenerationSpeed(20);
	this->circuits.SetGenerationSpeed(0);

	auto interactCollider = this->factory->CreateStaticGameObject<BoxCollider>();
	interactCollider->SetParent(this->GetPtr());
	// mark this collider as interactable so player raycasts can filter hits
	interactCollider->SetTag(Tag::INTERACTABLE | Tag::OBJECT);
	interactCollider->SetOnInteract([&](std::shared_ptr<Player> player) { this->Interact(player); });
	interactCollider->SetOnHover([&] { this->Hover(); });
	interactCollider->transform.SetScale(1, 1, 1);

	this->lastGenerated = Time::GetInstance().GetSessionTime();

	SoundClip* clip = AssetManager::GetInstance().GetSoundClip("SpaceshipAmbiance.wav");
	this->speaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
	this->speaker.lock()->SetParent(this->GetPtr());
	this->speaker.lock()->LoopSoundEffect(0); // 0 = loops forever
	this->speaker.lock()->SetRandomPitch(0.8f, 1.1f);
	this->speaker.lock()->SetGain(0.7f);
	this->speaker.lock()->Play(clip);

	SoundClip* buildClip = AssetManager::GetInstance().GetSoundClip("Build2.wav");
	std::weak_ptr<SoundSourceObject> tempSpeaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
	tempSpeaker.lock()->SetDeleteWhenFinnished(true);
	tempSpeaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
	tempSpeaker.lock()->SetRandomPitch(0.8f, 1.0f);
	tempSpeaker.lock()->SetGain(1.0f);
	tempSpeaker.lock()->Play(buildClip);

	this->MeshObject::Start();
}

void ResourceGenerator::Interact(std::shared_ptr<Player> player) {
	if (auto gameManager = GameManager::GetInstance(); gameManager && gameManager->GetInCombat()) {
		return;
	}

	if (InputManager::GetInstance().WasKeyPressed('R')) {
		auto parentWeak = this->GetParent();
		if (parentWeak.expired()) return;

		auto room = std::dynamic_pointer_cast<Room>(parentWeak.lock());
		if (!room) return;

		room->RemoveBuiltObject();

		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (!promptWeak.expired()) {
			auto prompt = promptWeak.lock();
			if (prompt) prompt->Hide();
		}

		return;
	}

	// player->resources.GetResource(this->resourceType).IncrementAmount(amountGenerated);
	player->resources.GetResource(ResourceType::Titanium).IncrementAmount(this->titanium.TakeCurrentlyGenerated(this->lastGenerated));
	player->resources.GetResource(ResourceType::CarbonFiber)
		.IncrementAmount(this->carbonFiber.TakeCurrentlyGenerated(this->lastGenerated));
	player->resources.GetResource(ResourceType::Circuit)
		.IncrementAmount(this->circuits.TakeCurrentlyGenerated(this->lastGenerated));
	player->resources.GetResource(ResourceType::Lubricant)
		.IncrementAmount(this->lubricant.TakeCurrentlyGenerated(this->lastGenerated));

	float currentTime = Time::GetInstance().GetSessionTime();

	this->lastGenerated = currentTime;

	// hide interaction prompt for a short time so text disappears when interacting
	auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
	if (!promptWeak.expired()) {
		auto prompt = promptWeak.lock();
		if (prompt) prompt->Hide();
	}

	// disable showing the prompt for 1 second
	this->hoverDisabledUntil = currentTime + 0.5f;
}

void ResourceGenerator::Hover() {
	float currentTime = Time::GetInstance().GetSessionTime();
	if (currentTime < this->hoverDisabledUntil) return;
	auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
	if (promptWeak.expired()) return;
	auto prompt = promptWeak.lock();
	if (!prompt) return;

	bool inCombat = GameManager::GetInstance() && GameManager::GetInstance()->GetInCombat();
	std::string txt = "Press \"F\" to collect resources";
	if (inCombat) {
		txt = "Can't collect during attacks";
	} else {
		txt += "\nPress \"R\" to discard generator";
	}
	DirectX::XMVECTOR worldPos = this->transform.GetGlobalPosition();
	prompt->Show(txt, worldPos);
}

size_t ResourceGenerator::GeneratorElement::GetCurrentlyGenerated(float lastGenerated) const {
	float currentTime = Time::GetInstance().GetSessionTime();
	float timePast = currentTime - lastGenerated;

	size_t amountGenerated = static_cast<size_t>((timePast) * this->speed / 60 + this->change);
	return amountGenerated;
}

float ResourceGenerator::GeneratorElement::GetGenerationSpeed() const { 
	return this->speed; }

void ResourceGenerator::GeneratorElement::SetGenerationSpeed(float matPerMin) { this->speed = matPerMin; }

size_t ResourceGenerator::GeneratorElement::TakeCurrentlyGenerated(float lastGenerated) { 
	float currentTime = Time::GetInstance().GetSessionTime();
	float timePast = currentTime - lastGenerated;

	float amountGenerated = timePast * this->speed / 60 + this->change;

	size_t amountGained = static_cast<size_t>(amountGenerated);

	this->change = amountGenerated - amountGained;

	return amountGained;
}
