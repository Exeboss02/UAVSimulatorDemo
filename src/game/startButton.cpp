#include "game/startButton.h"
#include "game/gameManager.h"

void StartButton::Start()
{
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

	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("EmergencyButton/Button.glb:Mesh_0"));

	this->SetName("Start button " + std::to_string(this->factory->GetNextID()));

	this->MeshObject::Start();
}

void StartButton::OnInteract(std::shared_ptr<Player> player)
{
    GameManager::GetInstance()->SpawnNextRound();
}
