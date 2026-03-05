#include "game/gunPickUp.h"
#include "UI/interactionPrompt.h"
#include <format>
GunPickUp::GunPickUp() {}

GunPickUp::~GunPickUp() {}

void GunPickUp::Start() {

	{
		auto meshobjweak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjweak.lock();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(this->gunVisualPath);
		meshobj->SetMesh(meshdata);
		meshobj->transform.SetScale(DirectX::XMLoadFloat3(&this->visualScale));
		meshobj->transform.SetRotationRPY(this->visualRotationRPY.x, this->visualRotationRPY.y,
										  this->visualRotationRPY.z);
		meshobj->SetCastShadow(false);
		this->gunVisual = meshobj;
	}
	
	{
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();

		DirectX::XMFLOAT3 pos(0.1f, 0.0f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		colliderobj->SetSolid(false);
		DirectX::XMFLOAT3 scale(0.9f, 0.3f, 0.1f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetOnInteract([&](std::shared_ptr<Player> playerShared) { this->OnInteract(playerShared); });
		colliderobj->SetOnHover([&] { this->Hover(); });
		colliderobj->SetTag(Tag::INTERACTABLE);
		//colliderobj->ShowDebug(true);
		//colliderobj->SetName("Interactable " + std::to_string(this->factory->GetNextID()));

		this->interactable = colliderobj;
	}
	//this->interactable.lock()->ShowDebug(true);
}

void GunPickUp::OnInteract(std::shared_ptr<Player> playerShared) {


	if (playerShared->resources.tryToPay(this->gunCost.getTitanium(), this->gunCost.getLubricant(), this->gunCost.getCarbonFiber(), this->gunCost.getCircuit())) {
		playerShared->addGun(Player::Guns::rifle);
		this->factory->QueueDeleteGameObject(this->GetPtr());
	}

}

void GunPickUp::Hover() {
	try {
		float currentTime = Time::GetInstance().GetSessionTime();
		if (currentTime < this->hoverDisabledUntil) return;
		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (promptWeak.expired()) return;
		auto prompt = promptWeak.lock();
		if (!prompt) return;

		std::string txt = std::format("Repair rifle, Cost {}", this->gunCost.getCostString());
		prompt->Show(txt);
	} catch (const std::exception& e) {
		Logger::Error("GunPickUp::Hover exception: ", e.what());
	} catch (...) {
		Logger::Error("GunPickUp::Hover unknown exception");
	}

}
