#include "game/startButton.h"
#include "game/gameManager.h"
#include "UI/interactionPrompt.h"

void StartButton::Start()
{
	auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
	collider->SetOnInteract([&](std::shared_ptr<Player> player) {
		this->OnInteract(player); 
	});
	collider->SetOnHover([&] { this->Hover(); });
	collider->SetParent(this->GetPtr());
	collider->transform.SetScale(4, 4, 4);

	this->transform.SetRotationRPY(3.14f, 3.14f, 1.57f);
	this->transform.SetScale(0.1, 0.1, 0.1);

	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("EmergencyButton/StartButton.glb:Mesh_0"));

	this->SetName("Start button " + std::to_string(this->factory->GetNextID()));

	this->MeshObject::Start();
}

void StartButton::OnInteract(std::shared_ptr<Player> player)
{
    if(!GameManager::GetInstance()->GetInCombat())
    {
        GameManager::GetInstance()->SpawnNextRound();
    }
}

void StartButton::Hover() { 
	auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
	std::shared_ptr<UI::InteractionPrompt> prompt = promptWeak.lock();

	if(prompt.get())
	{
		prompt->Show("Start next round");
	}
}
