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
    collider.lock()->SetBouncy(true);

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

    std::function<void(std::weak_ptr<GameObject3D>, std::weak_ptr<Collider>)> function = [&](std::weak_ptr<GameObject3D> gameObject3D, std::weak_ptr<Collider> collider)
    {
	    this->OnCollision(gameObject3D, collider);
    };

    this->SetAllOnCollisionFunction(function);
}

void FootBall::Tick()
{
    this->RigidBody::Tick();

    // DirectX::XMVECTOR pos = this->transform.GetGlobalPosition();
    // Logger::Error("pos: ", pos.m128_f32[0], ", ", pos.m128_f32[1], ", ", pos.m128_f32[2]);
}

void FootBall::OnCollision(std::weak_ptr<GameObject3D> gameObject3D, std::weak_ptr<Collider> collider)
{
    if(gameObject3D.expired()) return;

    auto rigidBody = std::dynamic_pointer_cast<RigidBody>(gameObject3D.lock());

    if(!rigidBody) return;

    if(collider.lock()->GetTag() & (Tag::FLOOR | Tag::PLAYER | Tag::WALL))
    {
        DirectX::XMVECTOR newVelocity = DirectX::XMLoadFloat3(&rigidBody->linearVelocity);
        newVelocity = DirectX::XMVectorScale(newVelocity, 8);
        DirectX::XMStoreFloat3(&this->linearVelocity, newVelocity);
    }
}