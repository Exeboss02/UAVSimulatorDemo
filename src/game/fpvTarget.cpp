#include "game/fpvTarget.h"

fpvTarget::fpvTarget() { 
	
}

fpvTarget::~fpvTarget() {}

void fpvTarget::Start() {

	GameObject3D::Start();
	{
	
		auto meshobjweak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjweak.lock();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(this->gunVisualPath);
		meshobj->SetMesh(meshdata);
		
		this->targetMesh = meshobj;
	

	
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();

		DirectX::XMFLOAT3 pos(0.0f, 0.0f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		colliderobj->SetSolid(false);
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(meshobj->GetPtr());
		colliderobj->SetOnCollision(
			[&](std::weak_ptr<GameObject> trigger, std::weak_ptr<Collider> collider) { this->OnExplode(); });
		
		// colliderobj->ShowDebug(true);
		// colliderobj->SetName("Interactable " + std::to_string(this->factory->GetNextID()));

		this->targetCollider = colliderobj;
	}
	this->targetCollider.lock()->ShowDebug(true);

}

void fpvTarget::OnExplode() {

	this->factory->QueueDeleteGameObject(this->GetPtr());

}
