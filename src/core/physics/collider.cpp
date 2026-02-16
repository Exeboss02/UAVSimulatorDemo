#include "core/physics/collider.h"
#include "core/physics/boxCollider.h"
#include "core/physics/sphereCollider.h"
#include "core/physics/rigidBody.h"
#include "core/physics/physicsQueue.h" //are here to prevent circular dependecies

Collider::Collider()
{
}

Collider::~Collider()
{
}

void Collider::SetParent(std::weak_ptr<GameObject> parent)
{
	std::shared_ptr<GameObject> newParent = parent.lock();

	if (!newParent)
	{
		Logger::Error("Tried to set expired gameobject as parent");
		return;
	}

	std::shared_ptr<Collider> colliderParent = std::dynamic_pointer_cast<Collider>(newParent);
	if(colliderParent != nullptr)
	{
		Logger::Error("Tried to set Collider as parent to a Collider");
		return;
	}

	std::shared_ptr<RigidBody> rigidBodyParent = std::dynamic_pointer_cast<RigidBody>(newParent);
	std::shared_ptr<GameObject3D> gameObject3DParent = std::dynamic_pointer_cast<GameObject3D>(newParent);
	std::shared_ptr<Collider> thisCollider = std::static_pointer_cast<Collider>(this->GetPtr());

	if (rigidBodyParent)
	{
		this->rigidBodyParent = rigidBodyParent;
		rigidBodyParent->AddColliderChild(thisCollider);
		PhysicsQueue::GetInstance().AddToAllColliders(thisCollider);
	}
	else if (gameObject3DParent)
	{
		this->gameObject3DParent = gameObject3DParent;
		PhysicsQueue::GetInstance().AddStrayCollider(thisCollider);
		PhysicsQueue::GetInstance().AddToAllColliders(thisCollider);
		Logger::Log("Added stray Collider with GameObject3D parent to physics queue");
	}
	else
	{
		Logger::Error("Tried to set GameObject as parent on Collider (parent has to be derived from GameObject3D)");
		return;
	} 

	this->GameObject3D::SetParent(newParent);
}

void Collider::Start()
{
	GameObject3D::Start();

	MeshObjData meshData = {};
	DirectX::XMVECTOR scale;
	scale.m128_f32[0] = 1;
	scale.m128_f32[1] = 1;
	scale.m128_f32[2] = 1;

	if(this->type == ColliderType::SPHERE)
	{
		scale = DirectX::XMVectorScale(scale, 0.5f);
		meshData = AssetManager::GetInstance().GetMeshObjData("meshes/indicatorSphere05.glb:Mesh_0");
	}
	else
	{
		meshData = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
	}

	auto material = AssetManager::GetInstance().GetMaterialWeakPtr("wireframeWhite");
	if(material.expired())
	{
		Logger::Error("Collider mesh material was expired");
		return;
	}

	meshData.SetMaterial(0, material.lock());
	auto visualMeshObject = this->factory->CreateGameObjectOfType<MeshObject>().lock();
	visualMeshObject->SetMesh(meshData);
	visualMeshObject->transform.SetScale(scale);
	visualMeshObject->SetParent(std::static_pointer_cast<Collider>(this->GetPtr()));
}

bool Collider::Collision(Collider* otherCollider)
{
	DirectX::XMFLOAT3 mtvAxis = {};
	float mtvDistance = 0;

	return this->CollisionHandling(otherCollider, mtvAxis, mtvDistance);
}

bool Collider::Collision(Collider* otherCollider, DirectX::XMVECTOR& contactNormal)
{
	DirectX::XMFLOAT3 mtvAxis = {};
	float mtvDistance = 0;
	bool collision = this->CollisionHandling(otherCollider, mtvAxis, mtvDistance);

	contactNormal = DirectX::XMLoadFloat3(&mtvAxis);
	contactNormal = DirectX::XMVector3Normalize(contactNormal);

	return collision;
}

void Collider::Tick()
{
	this->GameObject3D::Tick();
}

void Collider::SetId(int id) 
{
	this->id = id;
}

int Collider::GetId()
{ 
	return this->id; 
}

void Collider::ResolveCollision(DirectX::XMFLOAT3 resolveAxis, float resolveDistance)
{
	std::shared_ptr<RigidBody> rigidBodyParent = this->rigidBodyParent.lock();
	std::shared_ptr<GameObject3D> gameObject3DParent = this->gameObject3DParent.lock();
	GameObject3D* moveTarget = this;

	if (rigidBodyParent)
	{
		moveTarget = rigidBodyParent.get();
	} 
	else if (gameObject3DParent)
	{
		moveTarget = gameObject3DParent.get();
	}

	DirectX::XMFLOAT3 resolveVector = FLOAT3MULT1(resolveAxis, resolveDistance);
	DirectX::XMVECTOR moveDistance = DirectX::XMLoadFloat3(&resolveVector);

	DirectX::XMVECTOR oldPosition = moveTarget->transform.GetPosition();
	DirectX::XMVECTOR newPosition = DirectX::XMVectorAdd(oldPosition, moveDistance);

	moveTarget->transform.SetPosition(newPosition);
}

bool Collider::BoxSphereCollision(BoxCollider* box, SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance)
{
	using namespace DirectX;

	if (GetAsyncKeyState('B'))
	{
		if(sphere->targetTag == 3)
		{
			int a = 0;
		}
	}

	//CHECK MATRIX TRANSPOSE's!

	XMVECTOR boxCenter = box->GetGlobalPosition();
	XMVECTOR sphereCenter = sphere->GetGlobalPosition();
	XMFLOAT3 fExtents = FLOAT3MULT1(box->GetExtents(), 1);
	XMVECTOR vExtents = XMLoadFloat3(&fExtents);

	//inverse transpose?
	//XMMATRIX boxWorldMatrix = XMMatrixTranspose(box->GetGlobalWorldMatrix(false)); //worldMatrix is pre-transposed, so needs to get un-transposed again here
	//XMMATRIX boxWorldMatrix = box->GetGlobalWorldMatrix(false);
	XMFLOAT4X4 matrix = box->transform.GetWorldMatrix(false);
	XMMATRIX boxWorldMatrix = XMLoadFloat4x4(&matrix);

	//we get the inverse of the rotation and translation matrices, scale should not be included
	XMVECTOR scale, rotation, translation;
	XMMatrixDecompose(&scale, &rotation, &translation, boxWorldMatrix);

	XMMATRIX rotationMatrix = DirectX::XMMatrixRotationX(rotation.m128_f32[0]) * DirectX::XMMatrixRotationY(rotation.m128_f32[1]) * DirectX::XMMatrixRotationZ(rotation.m128_f32[2]);
	//XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation.m128_f32[0], rotation.m128_f32[1], rotation.m128_f32[2]);
	XMMATRIX invRotationMatrix = XMMatrixTranspose(rotationMatrix); //rotationMatrix here won't be pre-transposed, so a transpose is required to get the inverse
	XMMATRIX invTranslationMatrix = XMMatrixTranslation(-boxCenter.m128_f32[0], -boxCenter.m128_f32[1], -boxCenter.m128_f32[2]);
	XMMATRIX invScalingMatrix = XMMatrixScaling(1.0f / XMVectorGetX(scale), 1.0f / XMVectorGetY(scale), 1.0f / XMVectorGetZ(scale));

	XMMATRIX worldToLocalMatrix = invRotationMatrix * invTranslationMatrix;
	XMVECTOR vLocalSphereCenter = XMVector3TransformCoord(sphereCenter, worldToLocalMatrix);

	XMVECTOR closestPointOnBox = XMVectorClamp(vLocalSphereCenter, -vExtents, vExtents);
	XMVECTOR delta = XMVectorSubtract(vLocalSphereCenter, closestPointOnBox);

	float distSq = XMVectorGetX(XMVector3LengthSq(delta));
	float radius = sphere->GetDiameter() * 0.5f;

	if (distSq < radius * radius)
	{
		XMVECTOR axisLocal = XMVectorZero();
		float penetration = 0;

		if (box->dynamic)
		{
			int a = 0;
		}

		if (distSq < 1e-8f)
		{
			//this edge case needs to be handled
		}
		else
		{
			axisLocal = XMVector3Normalize(delta);
			penetration = radius - sqrtf(distSq);
		}

		XMMATRIX scalingMatrix = XMMatrixScaling(XMVectorGetX(scale), XMVectorGetY(scale), XMVectorGetZ(scale));
		XMMATRIX translationMatrix = XMMatrixTranslation(boxCenter.m128_f32[0], boxCenter.m128_f32[1], boxCenter.m128_f32[2]);
		XMMATRIX localToWorldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
		XMVECTOR axisWorld = XMVector3TransformNormal(axisLocal, localToWorldMatrix);
		XMStoreFloat3(&resolveAxis, XMVector3Normalize(axisWorld));
		resolveDistance = penetration;

		//std::cout << "-------------------------------------------------" << std::endl;
		//PrintFloat3(sphereCenter, "SphereCenter: ");
		//std::cout << "SphereRadius: " << radius << std::endl;
		//std::cout << "---" << std::endl;
		//PrintFloat3(boxCenter, "BoxCenter: ");
		//PrintFloat3(fExtents, "boxExtents(transform.scale)");

		//XMFLOAT4X4 world = {};
		//XMStoreFloat4x4(&world, boxWorldMatrix);
		//std::cout << "worldMatrix: ";
		//PrintMatrix(world);
		//std::cout << "---" << std::endl;
		//std::cout << "penetration: " << penetration << std::endl;
		//PrintFloat3(resolveAxis, "resolveAxis: ");
		//std::cout << "-------------------------------------------------" << std::endl;

		//Logger::Log("BOXSPHERE COLLISION!");

		return true;
	}

	//std::cout << "<<<<<not in collision>>>>>" << std::endl;

	return false;
}

bool Collider::CollisionHandling(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance)
{
	bool collision = false;
	collision = this->DoubleDispatchCollision(otherCollider, mtvAxis, mtvDistance);

	if (this->type == ColliderType::BOX && otherCollider->type == ColliderType::SPHERE)
	{
		mtvDistance = -mtvDistance;
	}

	if (!collision) return false;
	if (!this->solid || !otherCollider->solid) return collision;

	// Determine who moves
	if (this->dynamic && otherCollider->dynamic)
	{
		this->ResolveCollision(mtvAxis, mtvDistance / 2);
		otherCollider->ResolveCollision(mtvAxis, -mtvDistance / 2);
	}
	else if (this->dynamic)
	{
		this->ResolveCollision(mtvAxis, mtvDistance);
	}
	else if (otherCollider->dynamic)
	{
		otherCollider->ResolveCollision(mtvAxis, -mtvDistance);
	}

	return true;
}


