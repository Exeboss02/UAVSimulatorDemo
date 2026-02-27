#include "gameObjects/gun.h"
#include <numbers>
#include "gameObjects/rayVis.h"
#include "gameObjects/cameraObject.h"
#include "utilities/logger.h"
#include "core/physics/vector3D.h"
#include <memory>

Gun::Gun() {}

Gun::~Gun() {}

void Gun::Shoot(bool pressedTrigger, bool triggerDown) {

	switch (this->fireMode) {
	case (FireMode::SEMIAUTOMATIC):
		if (pressedTrigger == false) {
			return;
		}
		break;
	case (FireMode::AUTOMATIC):
		if (triggerDown == false) {
			return;
		}
		break;

	default:
		break;
	}

	

	if (!this->shootCoolDown.TimeIsUp()) {
		Logger::Log("cooldown not down");
		return;
	}
	this->shootCoolDown.Reset();
	

	std::shared_ptr<SoundSourceObject> lockedSpeaker = this->speaker.lock();
	lockedSpeaker->SetRandomPitch(0.92f, 1.0f);
	lockedSpeaker->Play(this->soundClips[0]); // shoot sound
	
	DirectX::XMVECTOR lookVec;
	DirectX::XMVECTOR posVec; 

	if (auto parentCameraShared  = this->parentCamera.lock()) {
		lookVec = this->parentCamera.lock()->transform.GetGlobalForward();
		posVec = this->parentCamera.lock()->transform.GetGlobalPosition();
	} else{
		lookVec = this->muzzle.lock()->transform.GetGlobalForward();
		posVec = this->muzzle.lock()->transform.GetGlobalPosition();
	}
	

	Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
	RayCastData rayCastData;

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, ~Tag::NOIGNORE, Tag::PLAYER);
	std::string hitString;
	if (didHit) {

		rayCastData.hitColider.lock()->Hit(this->damage);
		hitString = "hit";


		
		if (this->parentCamera.expired()) {
			this->VisulalizeShootBasedOnMuzzle(lookVec, posVec, rayCastData.distance);
		} else {
			this->VisualizeShootBasedOnCameraParent(lookVec, posVec, rayCastData.distance);
		}
		
	} else {
		hitString = "miss";
	}
	//Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));

}

void Gun::Start() {

	GameObject3D::Start();

	this->transform.SetPosition(DirectX::XMLoadFloat3(&this->gunPosition));

	this->shootCoolDown.Initialize(this->fireRate);

	// SFX
	this->speaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->speaker.lock()->SetParent(this->GetPtr());
	this->speaker.lock()->SetGain(1.0f);

	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Shoot3.wav"));


	{
		auto meshobjweak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjweak.lock();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(this->gunVisualPath);
		meshobj->SetMesh(meshdata);
		meshobj->transform.SetScale(DirectX::XMLoadFloat3(&this->visualScale));
		meshobj->transform.SetRotationRPY(this->visualRotationRPY.x, this->visualRotationRPY.y,
										  this->visualRotationRPY.z);
		this->gunVisual = meshobj;
	}

	{
		auto muzzleWeak = this->factory->CreateGameObjectOfType<GameObject3D>();

		auto muzzle = muzzleWeak.lock();

		muzzle->transform.SetPosition(DirectX::XMLoadFloat3(&this->muzzlePosition));

		muzzle->SetParent(this->GetPtr());

		this->muzzle = muzzle;
	}

}

void Gun::Tick() {

	GameObject3D::Tick();

	float deltaTime = Time::GetInstance().GetDeltaTime();
	if (deltaTime < 1) // to prevent tick spam when loading scene
	{
		this->shootCoolDown.Tick(deltaTime);
	}


}

void Gun::setParentToShootFrom(std::shared_ptr<GameObject3D> parentCamera) { this->parentCamera = parentCamera; }


void Gun::VisualizeShootBasedOnCameraParent(DirectX::XMVECTOR lookVec, DirectX::XMVECTOR posVec, float distance) {
	Vector3D hitPos(DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, distance)));
	Vector3D muzzlePos = Vector3D(this->muzzle.lock()->transform.GetGlobalPosition());
	Vector3D muzzleToHit = hitPos - muzzlePos;

	MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
	auto colliderobjWeak = this->factory->CreateGameObjectOfType<RayVis>();
	auto colliderobj = colliderobjWeak.lock();
	colliderobj->StartDeathTimer(0.05f);
	colliderobj->SetMesh(meshdata);
	colliderobj->GetMesh().SetMaterial(0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
	colliderobj->SetCastShadow(false);

	colliderobj->transform.SetPosition(
		DirectX::XMVectorAdd(muzzlePos.getXMVector(), DirectX::XMVectorScale(muzzleToHit.getXMVector(), 0.5)));

	//create rotation
	DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(muzzleToHit.getXMVector());

	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, up));
	if (fabsf(dot) > 0.99f) {
		up = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookToLH(DirectX::XMVectorZero(), direction, up);
	DirectX::XMMATRIX worldRotMat = XMMatrixTranspose(lookAtMat);

	DirectX::XMVECTOR quat = XMQuaternionRotationMatrix(worldRotMat);
	//create rotation end

	colliderobj->transform.SetRotationQuaternion(quat);

	DirectX::XMFLOAT3 scale(0.01f, 0.01f, muzzleToHit.Length() / 2);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));

}

void Gun::VisulalizeShootBasedOnMuzzle(DirectX::XMVECTOR lookVec, DirectX::XMVECTOR posVec, float distance) {
	// rayVis
	MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
	auto colliderobjWeak = this->factory->CreateGameObjectOfType<RayVis>();
	auto colliderobj = colliderobjWeak.lock();
	colliderobj->StartDeathTimer(0.05f);
	colliderobj->SetMesh(meshdata);
	colliderobj->GetMesh().SetMaterial(0,
									   AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
	colliderobj->SetCastShadow(false);
	colliderobj->transform.SetPosition(DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, distance / 2)));
	colliderobj->transform.SetRotationQuaternion(this->muzzle.lock()->transform.GetGlobalRotation());
	DirectX::XMFLOAT3 scale(0.01f, 0.01f, distance / 2);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	// end of rayVis
}