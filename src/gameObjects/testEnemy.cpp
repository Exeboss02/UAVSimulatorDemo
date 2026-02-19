#include "gameObjects/testEnemy.h"
#include <DirectXMath.h>

#include "core/assetManager.h"
#include "gameObjects/meshObject.h"

void TestEnemy::Start() {
	auto meshObjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

	auto meshObj = meshObjWeak.lock();
	meshObj->SetParent(this->GetPtr());

	MeshObjData meshData = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
	meshObj->SetMesh(meshData);

	auto collider = this->factory->CreateGameObjectOfType<BoxCollider>().lock();
	collider->transform.SetScale({1,1, 1});
	collider->SetParent(this->GetPtr());


	collider->SetOnInteract([&]() { 
		this->health -= 20;
		Logger::Log("Enemy got hit");
		if (this->health <= 0) {
			this->factory->QueueDeleteGameObject(this->GetPtr());
		}
	});

	this->hitBox = collider;
}

void TestEnemy::Tick() {
	if (!this->path.empty()) {

		if (this->currentPathIndex >= this->path.size()) {
			return;
		}

		if (DirectX::XMVector4NearEqual(this->transform.GetGlobalPosition(),
										this->path[this->currentPathIndex]->transform.GetGlobalPosition(),
										DirectX::XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f))) {
			this->currentPathIndex++;
			if (this->currentPathIndex >= this->path.size()) {
				return;
			}
		}

		DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(this->path[this->currentPathIndex]->GetGlobalPosition(),
																this->GetGlobalPosition());
		direction = DirectX::XMVector3Normalize(direction);
		this->transform.Move(direction, this->movementSpeed * Time::GetInstance().GetDeltaTime());
	}
}

void TestEnemy::SetPath(const std::vector<std::shared_ptr<AStarVertex>>& newPath) {
	this->path = newPath;
	this->currentPathIndex = 0;
	this->transform.SetPosition(this->path[0]->transform.GetGlobalPosition());
}
