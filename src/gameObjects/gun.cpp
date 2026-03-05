#include "gameObjects/gun.h"
#include "core/physics/vector3D.h"
#include "game/crosshair.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/rayVis.h"
#include "utilities/logger.h"
#include "utilities/time.h"
#include <memory>
#include <numbers>
#include <random>

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
			if (this->continuousFireTime > 0) {
				this->continuousFireTime -= Time::GetInstance().GetDeltaTime() * 3.0f;
			}  
			
			if (this->continuousFireTime < 0) {
				this->continuousFireTime = 0.0f;
			}
			return;
		} else {
		
		if (this->continuousFireTime <= this->timeToMaxBulletSpread) {
				continuousFireTime += Time::GetInstance().GetDeltaTime();
			}
		
		}
		break;

	default:
		break;
	}

	if (!this->shootCoolDown.TimeIsUp()) {
		return;
	}

	this->shootCoolDown.Reset();

	SoundClip* shootClip = AssetManager::GetInstance().GetSoundClip(this->soundClipShootPath);
	std::shared_ptr<SoundSourceObject> lockedSpeaker = this->speaker.lock();
	lockedSpeaker->SetRandomPitch(0.92f, 1.0f);
	lockedSpeaker->Play(shootClip); // shoot sound

	DirectX::XMVECTOR lookVec;
	DirectX::XMVECTOR posVec;
	std::shared_ptr<GameObject3D> muzzleLocked = this->muzzle.lock();

	//bullet spread
	switch (this->fireMode) {
	case (FireMode::SEMIAUTOMATIC):
		lookVec = this->muzzle.lock()->transform.GetGlobalForward();
		break;
	case (FireMode::AUTOMATIC): 
	{
		float bulletSpread = this->maxBulletSpread * (this->continuousFireTime / this->timeToMaxBulletSpread);
		float randomP = this->GetRandomValueInRange(0, bulletSpread * 2);
		float randomY = this->GetRandomValueInRange(0, bulletSpread * 2);
		muzzleLocked->transform.SetRotationRPY(0, (randomP - bulletSpread) * std::numbers::pi / 180,
											   (randomY - bulletSpread) * std::numbers::pi / 180);
		lookVec = this->muzzle.lock()->transform.GetGlobalForward();
		muzzleLocked->transform.SetRotationRPY(0, 0, 0);

		break;
	}

	default:
		break;
	}
	

	if (auto parentCameraShared = this->parentCamera.lock()) {
		posVec = this->parentCamera.lock()->transform.GetGlobalPosition();
	} else {
		posVec = this->muzzle.lock()->transform.GetGlobalPosition();
	}

	Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
	RayCastData rayCastData;

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, ~Tag::NOIGNORE, Tag::PLAYER | Tag::FRIENDLY);
	std::string hitString;
	if (didHit) {
		auto hitCollider = rayCastData.hitColider.lock();
		if (hitCollider) {
			hitCollider->Hit(this->damage);
			if ((hitCollider->GetTag() & Tag::ENEMY) != 0) {
				auto crosshairWeak = this->factory->FindObjectOfType<Crosshair>();
				if (!crosshairWeak.expired()) {
					if (auto crosshair = crosshairWeak.lock()) {
						crosshair->ShowHitIndicator();
					}
				}
			}
		}
		hitString = "hit";

		if (this->parentCamera.expired()) {
			this->VisulalizeShootBasedOnMuzzle(lookVec, posVec, rayCastData.distance);
		} else {
			this->VisualizeShootBasedOnCameraParent(lookVec, posVec, rayCastData.distance);
		}

	} else {
		hitString = "miss";
	}
	// Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
}

void Gun::Start() {

	GameObject3D::Start();

	this->transform.SetPosition(DirectX::XMLoadFloat3(&this->gunPosition));

	this->shootCoolDown.Initialize(this->fireRate);

	// SFX
	this->speaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->speaker.lock()->SetParent(this->GetPtr());

	if (this->fireMode == FireMode::SEMIAUTOMATIC) {
		this->speaker.lock()->SetGain(0.6f);
	} else {
		this->speaker.lock()->SetGain(0.3f);
	}

	{
		auto meshobjweak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjweak.lock();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(this->gunVisualPath);
		meshobj->SetMesh(meshdata);
		meshobj->transform.SetScale(DirectX::XMLoadFloat3(&this->visualScale));
		meshobj->transform.SetRotationRPY(this->visualRotationRPY.x, this->visualRotationRPY.y,
										  this->visualRotationRPY.z);
		meshobj->SetCastShadow(false);
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
	colliderobj->GetMesh().SetMaterial(0,
									   AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
	colliderobj->SetCastShadow(false);

	colliderobj->transform.SetPosition(
		DirectX::XMVectorAdd(muzzlePos.getXMVector(), DirectX::XMVectorScale(muzzleToHit.getXMVector(), 0.5)));

	// create rotation
	DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(muzzleToHit.getXMVector());

	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, up));
	if (fabsf(dot) > 0.99f) {
		up = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookToLH(DirectX::XMVectorZero(), direction, up);
	DirectX::XMMATRIX worldRotMat = XMMatrixTranspose(lookAtMat);

	DirectX::XMVECTOR quat = XMQuaternionRotationMatrix(worldRotMat);
	// create rotation end

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

float Gun::GetRandomValueInRange(float min, float max) {
	static std::mt19937 gen(std::random_device{}());

	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}
