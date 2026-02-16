#include "gameObjects/wall.h"
void Wall::OnObserve() {

}

void Wall::OnInteract() {

}

void Wall::Start(){ 
	
	GameObject::Start();

	auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();

	auto colliderobj = colliderobjWeak.lock();
	DirectX::XMFLOAT3 pos(4.5f, 3.0f, 0.0f);
	colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	DirectX::XMFLOAT3 scale(0.250f, 0.750f, 0.750f);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	colliderobj->SetParent(this->GetPtr());

	this->interactable = colliderobj;

}