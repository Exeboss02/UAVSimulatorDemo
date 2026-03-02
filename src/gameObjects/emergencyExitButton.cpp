#include "gameObjects/emergencyExitButton.h"
#include "game/gameManager.h"

void EmergenceExitButton::Start() { 
	auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
	collider->SetOnInteract([&](std::shared_ptr<Player> player) {
		this->OnInteract(player); 
	});
	collider->SetParent(this->GetPtr());
	collider->transform.SetScale(4, 4, 4);

	this->transform.SetRotationRPY(3.14/2, 3.14, 0);
	this->transform.SetScale(0.1, 0.1, 0.1);
	auto sign = this->factory->CreateStaticGameObject<MeshObject>();
	sign->SetParent(this->GetPtr());
	sign->transform.SetPosition(-6, -0.5, 0);
	sign->transform.SetScale(2, 2, 2);
	this->sign = sign;
	this->SetState(EmergenceExitButton::State::PreTouchUp);

	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("EmergencyButton/Button.glb:Mesh_0"));

	this->SetName("Emergency Button " + std::to_string(this->factory->GetNextID()));

	this->MeshObject::Start();
}

void EmergenceExitButton::SetState(EmergenceExitButton::State state) {
	if (this->sign.expired()) return;
	auto sign = this->sign.lock();

	switch (state) {
	case (EmergenceExitButton::State::PreTouchUp):
		sign->SetMesh(AssetManager::GetInstance().GetMeshObjData("EmergencyButton/EmergencySign1.glb:Mesh_0"));
		break;
	case (EmergenceExitButton::State::PostTouchUp):
		sign->SetMesh(AssetManager::GetInstance().GetMeshObjData("EmergencyButton/EmergencySign2.glb:Mesh_0"));
		break;
	}

	this->state = state;
}

void EmergenceExitButton::OnInteract(std::shared_ptr<Player> player) { 
	switch (this->state) {
	case (EmergenceExitButton::State::PreTouchUp): {
		GameManager::GetInstance()->PlayerDied();
		break;
	}
	case (EmergenceExitButton::State::PostTouchUp):
		GameManager::GetInstance()->Win();
		break;
	}

}
