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
	
	GameObject::Start();

	auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();

	auto colliderobj = colliderobjWeak.lock();
	DirectX::XMFLOAT3 pos(0.0f, 3.0f, 4.5f);
	colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	DirectX::XMFLOAT3 scale(0.750f, 0.750f, 0.250f);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	colliderobj->SetParent(this->GetPtr());

	this->interactable = colliderobj;

}

void Wall::SetWAllIndex(int wallIndex) { this->wallIndex = wallIndex; }