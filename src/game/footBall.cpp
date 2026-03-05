#include "game/footBall.h"
#include "game/gameManager.h"

FootBall::FootBall() {}
FootBall::~FootBall() {}

void FootBall::Start()
{
    this->RigidBody::Start();

    auto collider = this->factory->CreateGameObjectOfType<SphereCollider>();
	collider.lock()->SetDynamic(true);
	collider.lock()->SetParent(this->GetPtr());
	collider.lock()->SetSolid(true);

    this->gravity = true;

    auto meshData = AssetManager::GetInstance().GetMeshObjData("meshes/indicatorSphere05.glb:Mesh_0");

    auto visualMeshObject = this->factory->CreateGameObjectOfType<MeshObject>();
    visualMeshObject.lock()->SetMesh(meshData);
    visualMeshObject.lock()->SetParent(this->GetPtr());
    visualMeshObject.lock()->SetActive(true);
    visualMeshObject.lock()->SetCastShadow(true);
    visualMeshObject.lock()->transform.SetScale(0.5f, 0.5f, 0.5f);

    DirectX::XMVECTOR offset = {};
    offset.m128_f32[0] = -1;
    offset.m128_f32[2] = 0;
	this->transform.SetPosition(DirectX::XMVectorAdd(GameManager::GetInstance()->GetPlayerSpawnPoint(), offset));
}

void FootBall::Tick()
{
    this->RigidBody::Tick();

    DirectX::XMVECTOR pos = this->transform.GetGlobalPosition();
    Logger::Error("pos: ", pos.m128_f32[0], ", ", pos.m128_f32[1], ", ", pos.m128_f32[2]);
}