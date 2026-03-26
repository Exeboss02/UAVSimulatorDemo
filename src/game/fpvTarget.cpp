#include "game/fpvTarget.h"
#include <numbers>
#include "game/drone.h"
fpvTarget::fpvTarget() { 
	
}

fpvTarget::~fpvTarget() {}

void fpvTarget::Start() {

	GameObject3D::Start();
	{
		this->transform.SetRotationRPY(0, 0.588, 0);
		auto meshobjweak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjweak.lock();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(this->gunVisualPath);
		meshobj->SetMesh(meshdata);
		
		this->targetMesh = meshobj;
	

	
		auto colliderobj = this->factory->CreateGameObjectOfType<BoxCollider>().lock();
		colliderobj->SetDynamic(true);
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

void fpvTarget::Tick() { this->move();

}

void fpvTarget::OnExplode() {
	std::shared_ptr<FPVDrone> fpv = this->factory->FindObjectOfType<FPVDrone>().lock();

	fpv->transform.SetPosition(50, 10, 50);

	this->factory->QueueDeleteGameObject(this->GetPtr());

}

Vector3D fpvTarget::Lerp(const Vector3D& start, const Vector3D& end, float val) { return start + (end - start) * val; }

void fpvTarget::move() {

	this->timer += Time::GetInstance().GetDeltaTime();

	if (this->timer > this->hitTime) {
		
		
		
		this->factory->QueueDeleteGameObject(this->GetPtr());
	}

	this->transform.SetPosition(this->Lerp(this->startPos, this->endPos, this->timer / this->hitTime).getXMVector());


}

