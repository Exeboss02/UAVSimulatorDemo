#include "game/gunPickUp.h"


GunPickUp::GunPickUp() {}

GunPickUp::~GunPickUp() {}

void GunPickUp::Start() {

	{
		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("guns/GunTest.glb:Mesh_0");
		this->SetMesh(meshdata);
	}
	
	{
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();

		/*DirectX::XMFLOAT3 pos(0.0f, 3.0f, 4.5f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));*/
		/*DirectX::XMFLOAT3 scale(0.750f, 0.750f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));*/
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetOnInteract([&](std::shared_ptr<Player> playerShared) { this->OnInteract(playerShared); });
		colliderobj->SetOnHover([&] { this->Hover(); });
		colliderobj->SetTag(Tag::INTERACTABLE);
		//colliderobj->SetName("Interactable " + std::to_string(this->factory->GetNextID()));

		this->interactable = colliderobj;
	}

}

void GunPickUp::OnInteract(std::shared_ptr<Player> playerShared) {
	
	
	Logger::Log("give me gun!");


}

void GunPickUp::Hover() {


}
