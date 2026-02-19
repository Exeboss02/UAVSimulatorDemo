#include "gameObjects/wall.h"
#include "utilities/logger.h"
#include "gameObjects/room.h"
void Wall::OnObserve() { }

void Wall::OnInteract() { 
	
	Logger::Log("stop interacting with me!"); 

	auto parentWeak = this->GetParent();
	if (parentWeak.expired()) {
		Logger::Error("wall parent is dead, how in the funk?");
		return;
	}

	auto parent = std::static_pointer_cast<Room>(parentWeak.lock());

	
	parent->CreateRoom(static_cast<Room::WallIndex>(this->wallIndex));
}

void Wall::Start(){ 
	this->MeshObject::Start();
	Logger::Log("Wall started");
	this->interactable.lock()->SetOnInteract([&]() { 
		this->OnInteract(); 
		});
}

void Wall::SetWAllIndex(int wallIndex) { this->wallIndex = wallIndex; }


void Wall::SpawnInteractables() {
	auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();

	auto colliderobj = colliderobjWeak.lock();
	DirectX::XMFLOAT3 pos(0.0f, 3.0f, 4.5f);
	colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	DirectX::XMFLOAT3 scale(0.750f, 0.750f, 0.250f);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	colliderobj->SetParent(this->GetPtr());

	this->interactable = colliderobj;
}

void Wall::SpawnWallColliders(int wallStateIndex) {
	for (auto colliders: this->wallColliders) {
		this->factory->QueueDeleteGameObject(colliders);
	}
	this->wallColliders.clear();
	Room::WallState wallState = static_cast<Room::WallState>(wallStateIndex);
	switch (wallState) {
	case (Room::WallState::window || Room::WallState::solid):
		{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();
		auto colliderobj = colliderobjWeak.lock();
		DirectX::XMFLOAT3 pos(0.0f, 3.0f, 4.75f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(5.0f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());

		this->wallColliders.push_back(colliderobj);
	}
		break;

	case (Room::WallState::door): {
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();
		auto colliderobj = colliderobjWeak.lock();
		DirectX::XMFLOAT3 pos(3.25f, 3.0f, 4.75f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.75f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());

		this->wallColliders.push_back(colliderobj);
	}
	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();
		auto colliderobj = colliderobjWeak.lock();
		DirectX::XMFLOAT3 pos(-3.25f, 3.0f, 4.75f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.75f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());

		this->wallColliders.push_back(colliderobj);
	}
		break;

	default:
		break;
	}
}

void Wall::SetWallState(int wallState) {

	MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(
	Wall::wallMeshIdentifiers[wallState]);
	this->SetMesh(meshdata);
	this->SpawnWallColliders(wallState);
	Room::WallState wallStateEnum = static_cast<Room::WallState>(wallState);
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

void Wall::RemoveInteractables() {

	this->factory->QueueDeleteGameObject(this->interactable);
}