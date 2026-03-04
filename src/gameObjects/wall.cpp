#include "gameObjects/wall.h"
#include "UI/interactionPrompt.h"
#include "game/gameManager.h"
#include "gameObjects/room.h"
#include "utilities/logger.h"
#include "utilities/time.h"
void Wall::OnObserve() {}

void Wall::OnInteract() {
	try {
		if (auto gameManager = GameManager::GetInstance(); gameManager && gameManager->GetInCombat()) {
			return;
		}
	} catch (...) {
	}
}

void Wall::Interact(std::shared_ptr<Player> playerShared) {

	if (GameManager::GetInstance()->GetInCombat()) {
		return;
	}

	if (!playerShared->resources.tryToPay(this->wallCost.getTitanium(), this->wallCost.getLubricant(),
										  this->wallCost.getCarbonFiber(), this->wallCost.getCircuit())) {
		return;
	}

	auto parentWeak = this->GetParent();
	if (parentWeak.expired()) {
		Logger::Error("wall parent is dead, how in the funk?");
		return;
	}

	auto parent = std::static_pointer_cast<Room>(parentWeak.lock());

	parent->CreateRoom(static_cast<Room::WallIndex>(this->wallIndex));

	// hide shared interaction prompt briefly so text disappears on interact
	try {
		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (!promptWeak.expired()) {
			auto prompt = promptWeak.lock();
			if (prompt) prompt->Hide();
		}
	} catch (const std::exception& e) {
		Logger::Error("Wall::OnInteract Hide prompt exception: ", e.what());
	} catch (...) {
		Logger::Error("Wall::OnInteract Hide prompt unknown exception");
	}

	// disable showing the prompt for a short time
	this->hoverDisabledUntil = Time::GetInstance().GetSessionTime() + 0.5f;
}

void Wall::Start() {
	this->MeshObject::Start();
	Logger::Log("Wall started");

	this->SetName("WALL" + std::to_string(this->factory->GetNextID()));
}

void Wall::SetWAllIndex(int wallIndex) { this->wallIndex = wallIndex; }

void Wall::SpawnInteractables() {
	auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();

	DirectX::XMFLOAT3 pos(0.0f, 3.0f, 4.5f);
	colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	DirectX::XMFLOAT3 scale(3.0f, 3.0f, 0.250f);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	colliderobj->SetParent(this->GetPtr());
	colliderobj->SetOnInteract([&](std::shared_ptr<Player> playerShared) { this->Interact(playerShared); });
	colliderobj->SetOnHover([&] { this->Hover(); });
	colliderobj->SetTag(Tag::INTERACTABLE);
	colliderobj->SetName("Interactable " + std::to_string(this->factory->GetNextID()));

	this->interactable = colliderobj;
}

void Wall::SpawnWallColliders(int wallStateIndex) {
	for (auto colliders : this->wallColliders) {
		this->factory->QueueDeleteGameObject(colliders);
	}
	this->wallColliders.clear();
	Room::WallState wallState = static_cast<Room::WallState>(wallStateIndex);
	switch (wallState) {
	case (Room::WallState::window):
	case (Room::WallState::solid): {
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
		DirectX::XMFLOAT3 pos(0.0f, 3.0f, 4.75f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(5.0f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::WALL);

		this->wallColliders.push_back(colliderobj);
	} break;

	case (Room::WallState::door): {
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
		DirectX::XMFLOAT3 pos(3.25f, 3.0f, 4.75f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.75f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::WALL);

		this->wallColliders.push_back(colliderobj);
	}
		{
			auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
			DirectX::XMFLOAT3 pos(-3.25f, 3.0f, 4.75f);
			colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
			DirectX::XMFLOAT3 scale(1.75f, 2.5f, 0.250f);
			colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
			colliderobj->SetParent(this->GetPtr());
			colliderobj->SetTag(Tag::WALL);

			this->wallColliders.push_back(colliderobj);
		}
		break;

	default:
		break;
	}
}

void Wall::SetWallState(int wallState, bool edgeWall) {

	MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(Wall::wallMeshIdentifiers[wallState]);
	this->SetMesh(meshdata);
	this->SpawnWallColliders(wallState);
	Room::WallState wallStateEnum = static_cast<Room::WallState>(wallState);
	if (edgeWall) {
		return;
	}
	switch (wallStateEnum) {
	case (Room::WallState::window):
		this->SpawnInteractables();
		break;

	case (Room::WallState::solid):
		this->SpawnInteractables();
		break;

	case (Room::WallState::door):
		this->RemoveInteractables();
		break;

	default:
		break;
	}
}

void Wall::SetWallCost(size_t titanium, size_t lubricant, size_t carbonfiber, size_t circuitboards)
{
	this->wallCost.setTitanium(titanium);
	this->wallCost.setLubricant(lubricant);
	this->wallCost.setCarbonFiber(carbonfiber);
	this->wallCost.setCircuit(circuitboards);
}

void Wall::RemoveInteractables() {
	if (!this->interactable.expired()) {
		this->factory->QueueDeleteGameObject(this->interactable);
	}
}

void Wall::Hover() {
	try {
		float currentTime = Time::GetInstance().GetSessionTime();
		if (currentTime < this->hoverDisabledUntil) return;
		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (promptWeak.expired()) return;
		auto prompt = promptWeak.lock();
		if (!prompt) return;

		std::string txt = std::format("Press \"F\" to build room: Cost {}", this->wallCost.getCostString());
		try {
			if (auto gameManager = GameManager::GetInstance(); gameManager && gameManager->GetInCombat()) {
				txt = "Can't build during attacks";
			}
		} catch (...) {
		}
		prompt->Show(txt);
	} catch (const std::exception& e) {
		Logger::Error("Wall::Hover exception: ", e.what());
	} catch (...) {
		Logger::Error("Wall::Hover unknown exception");
	}
}