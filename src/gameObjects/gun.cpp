#include "gameObjects/gun.h"
#include <numbers>
#include "gameObjects/rayVis.h"

Gun::Gun() {}

Gun::~Gun() {}

void Gun::Shoot() {

	if (!this->shootCoolDown.TimeIsUp()) {
		Logger::Log("cooldown not down");
		return;
	}
	this->shootCoolDown.Reset();


	//std::shared_ptr<SoundSourceObject> lockedSpeaker = this->speaker.lock();
	//lockedSpeaker->Play(this->soundClips[0]); // shoot sound
	

	const DirectX::XMVECTOR lookVec = this->muzzle.lock()->transform.GetGlobalForward();
	const DirectX::XMVECTOR posVec = this->muzzle.lock()->transform.GetGlobalPosition();

	Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
	RayCastData rayCastData;

	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData);
	std::string hitString;
	if (didHit) {

		rayCastData.hitColider.lock()->Hit(this->damage);
		hitString = "hit";

		// rayVis
		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<RayVis>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->StartDeathTimer(0.05f);
		colliderobj->SetMesh(meshdata);
		colliderobj->GetMesh().SetMaterial(
			0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
		colliderobj->transform.SetPosition(
			DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, rayCastData.distance / 2)));
		colliderobj->transform.SetRotationQuaternion(this->muzzle.lock()->transform.GetGlobalRotation());
		DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance / 2);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		// end of rayVis
	} else {
		hitString = "miss";
	}
	//Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));

}

void Gun::Start() {

	GameObject3D::Start();

	this->transform.SetPosition(DirectX::XMLoadFloat3(&this->gunPosition));

	this->musicTimer.Initialize(2);
	this->sfxTimer.Initialize(0.4f);
	this->shootCoolDown.Initialize(this->fireRate);

	// SFX
	/*this->speaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->speaker.lock()->SetParent(this->GetPtr());
	this->speaker.lock()->SetGain(1.0f);

	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Shoot3.wav"));*/


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