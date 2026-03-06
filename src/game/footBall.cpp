#include "game/footBall.h"
#include "game/gameManager.h"

FootBall::FootBall() {}
FootBall::~FootBall() {}

void FootBall::Start()
{
    this->RigidBody::Start();

    this->collider = this->factory->CreateGameObjectOfType<SphereCollider>();
	this->collider.lock()->SetDynamic(true);
	this->collider.lock()->SetParent(this->GetPtr());
	this->collider.lock()->SetSolid(true);
    this->collider.lock()->SetBouncy(true);

    this->gravity = true;

    auto meshData = AssetManager::GetInstance().GetMeshObjData("meshes/ball.glb:Mesh_0");

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

void FootBall::PhysicsTick()
{
    //Rotation, should maybe be moved to rigidbody logic with lock axis bool's
    float radius = this->collider.lock()->GetDiameter();
    this->angularVelocity = DirectX::XMFLOAT3(this->linearVelocity.z * (1 / radius) * 50, 0, -this->linearVelocity.x * (1 / radius) * 50);

    this->RigidBody::PhysicsTick();
}

void FootBall::OnCollision(std::weak_ptr<GameObject3D> gameObject3D, std::weak_ptr<Collider> collider)
{
    if(gameObject3D.expired()) return;

    auto rigidBody = std::dynamic_pointer_cast<RigidBody>(gameObject3D.lock());

    if(!rigidBody) return;

    if(collider.lock()->GetTag() & Tag::PLAYER)
    {
        DirectX::XMVECTOR newVelocity = DirectX::XMVectorSubtract(this->transform.GetGlobalPosition(), rigidBody->transform.GetGlobalPosition());
        newVelocity = DirectX::XMVector3Normalize(newVelocity);
        newVelocity = DirectX::XMVectorScale(newVelocity, 0.3f);
        newVelocity.m128_f32[1] += 0.2f;
        DirectX::XMStoreFloat3(&this->linearVelocity, newVelocity);
    }
}