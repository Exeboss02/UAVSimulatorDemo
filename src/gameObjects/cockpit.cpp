#include "gameObjects/cockpit.h"
#include "core/physics/sphereCollider.h"
#include "game/gameManager.h"
#include "game/gunPickUp.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/emergencyExitButton.h"
#include "gameObjects/room.h"
#include "utilities/aStar.h"
#include <numbers>
#include "gameObjects/spotlightObject.h"
#include "game/footBall.h"

void Cockpit::Start() {
	this->SetName("Cockpit");
	auto coreCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	coreCollider->SetParent(this->GetPtr());
	coreCollider->SetSolid(true);
	coreCollider->SetTag(Tag::PLAYER);

	auto coreMesh = this->factory->CreateStaticGameObject<MeshObject>();
	coreMesh->SetMesh(AssetManager::GetInstance().GetMeshObjData("meshes/core.glb:Mesh_0"));
	coreMesh->SetParent(this->GetPtr());
	coreMesh->transform.SetPosition(0, 2, 0);
	coreMesh->transform.SetScale(1, 1.2f, 1);


	//FootBall
    DirectX::XMVECTOR offset = {};
    offset.m128_f32[0] = -1;
    offset.m128_f32[2] = 0;
	auto footBall = this->factory->CreateStaticGameObject<FootBall>();
	footBall->transform.SetPosition(DirectX::XMVectorAdd(GameManager::GetInstance()->GetPlayerSpawnPoint(), offset));
	footBall->transform.SetScale(1.0f, 1.0f, 1.0f);

	// Update core health on hud
	GameManager::GetInstance()->GetPlayer()->hud->SetCoreHealthText(this->health.Get());

	coreCollider->SetOnHit([&](float damage) {
		this->health.Decrement(damage);
		if (this->health.IsDead()) {
			this->onDeath();
			Logger::Log("Core Died");

			// create tempspeaker
			SoundClip* deathClip = AssetManager::GetInstance().GetSoundClip("BigExplosion2.wav");
			std::weak_ptr<SoundSourceObject> deathSpeaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
			deathSpeaker.lock()->SetDeleteWhenFinnished(true);
			deathSpeaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
			deathSpeaker.lock()->Play(deathClip);
			GameManager::GetInstance()->Loose();
		}

		// Update core health on hud
		GameManager::GetInstance()->GetPlayer()->hud->SetCoreHealthText(this->health.Get());

		SoundClip* damageClip = AssetManager::GetInstance().GetSoundClip("SmallExplotion.wav");
		this->speaker.lock()->SetDeleteWhenFinnished(true);
		this->speaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
		this->speaker.lock()->Play(damageClip);
	});

	coreCollider->transform.SetPosition(0, 2, 0);
	coreCollider->transform.SetScale(2, 2, 2);

	this->corePosition = coreCollider->transform.GetGlobalPosition();

	// this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	// auto tempCockpitMesh = this->factory->CreateStaticGameObject<MeshObject>();
	// tempCockpitMesh->SetParent(this->GetPtr());
	// tempCockpitMesh->transform.SetPosition(0, 3, 0);
	// tempCockpitMesh->transform.SetScale(5, 2.5, 5);
	// tempCockpitMesh->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	this->speaker = this->factory->CreateStaticGameObject<SoundSourceObject>();

	auto emergencyButton = this->factory->CreateStaticGameObject<EmergenceExitButton>();
	emergencyButton->transform.SetPosition(4.43, 2, 2);
	emergencyButton->SetParent(this->GetPtr());

	auto gunPickUp = this->factory->CreateStaticGameObject<GunPickUp>();
	gunPickUp->transform.SetPosition(-4, 1.45, 2.3);
	gunPickUp->transform.SetRotationRPY(0, 3.14 / 2, 3.14 * 1.75);

	auto globalScale = this->transform.GetGlobalScale();

	// Scaling down from spaceship scaling
	gunPickUp->transform.SetScale(1 / globalScale.m128_f32[0], 1 / globalScale.m128_f32[1],
								  1 / globalScale.m128_f32[2]);

	gunPickUp->SetParent(this->GetPtr());

	auto testRoom = this->factory->CreateStaticGameObject<MeshObject>();
	testRoom->SetMesh(AssetManager::GetInstance().GetMeshObjData("SpaceShip/SpaceshipCockpit.glb:Mesh_0"));
	testRoom->SetParent(this->GetPtr());
	testRoom->transform.SetScale(1 / globalScale.m128_f32[0], 1 / globalScale.m128_f32[1],
								  1 / globalScale.m128_f32[2]);
	testRoom->transform.SetRotationRPY({0,-DirectX::XM_PIDIV2,0});
	testRoom->transform.SetPosition(0, 0.5, 0);

	auto spotLight = this->factory->CreateGameObjectOfType<SpotlightObject>().lock();
	spotLight->SetParent(this->GetPtr());
	spotLight->transform.SetPosition({0, 4.5, 2});
	spotLight->transform.SetRotationRPY(0, std::numbers::pi / 2, 0);
	spotLight->SetAngle(140.);
	spotLight->SetIntensity(30.0f);



	this->GameObject3D::Start();

	this->createVisualsAndColiders();
}

void Cockpit::SetOnDeath(std::function<void()> func) { this->onDeath = func; }

void Cockpit::SetupPathfindingNodes(std::shared_ptr<SpaceShip> spaceShip) {

	size_t nodeCount = this->pathfindingNodes.size();

	// Core node
	auto vertex = this->factory->CreateGameObjectOfType<AStarVertex>().lock();
	vertex->SetParent(this->GetPtr());

	DirectX::XMVECTOR localCorePosition = DirectX::XMVectorSet(0, 2, 0, 0);
	vertex->Initialize(this->corePosition);
	vertex->transform.SetPosition(localCorePosition);

	this->pathfindingNodes[0] = vertex;
	spaceShip->GetPathfinder()->AddVertex(vertex);

	// Door node
	auto doorVertex = this->factory->CreateGameObjectOfType<AStarVertex>().lock();
	doorVertex->SetParent(this->GetPtr());

	DirectX::XMVECTOR doorPosition = DirectX::XMVectorSet(0, 2, 3, 0);
	doorVertex->Initialize(DirectX::XMVectorAdd(this->corePosition, DirectX::XMVectorSet(0, 0, 3, 0)));
	doorVertex->transform.SetPosition(doorPosition);

	this->pathfindingNodes[1] = doorVertex;
	spaceShip->GetPathfinder()->AddVertex(doorVertex);

	// Edge
	spaceShip->GetPathfinder()->AddEdge(this->pathfindingNodes[0], this->pathfindingNodes[1], 1);

	// Set pathfinding goal to core
	spaceShip->GetPathfinder()->SetGoal(this->pathfindingNodes[0]);
}

void Cockpit::createVisualsAndColiders() {

	// Walls
	for (int i = 0; i < 2; i++) {
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
		DirectX::XMFLOAT3 pos(4.75f * (i == 0 ? 1 : -1), 3.0f, 0);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(5.0f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->transform.SetRotationRPY(0, 0, std::numbers::pi / 2);
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::WALL);
		//auto col = colliderobj.Get();
		//colliderobj.Init();
		//col->ShowDebug(true);
	}

	// Control table
	{
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
		DirectX::XMFLOAT3 pos(0, 3.0f, -3);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(5.0f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::WALL);
		//auto col = colliderobj.Get();
		//colliderobj.Init();
		//col->ShowDebug(true);
	}

	// Angled walls
	for (int i = 0; i < 2; i++) {
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
		DirectX::XMFLOAT3 pos(4.75f * (i == 0 ? 1 : -1), 3.0f, 0);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(5.0f, 2.5f, 0.250f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->transform.SetRotationRPY(0, 0, DirectX::XM_PIDIV4 * (i == 0 ? -1 : 1));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::WALL);
		//auto col = colliderobj.Get();
		//colliderobj.Init();
		//col->ShowDebug(true);
	}

	// Table
	{
		auto colliderobj = this->factory->CreateStaticGameObject<BoxCollider>();
		DirectX::XMFLOAT3 pos(-4, 0.9f, 2.3f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.0f, 0.5f, 0.5f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->transform.SetRotationRPY(0, 0, DirectX::XM_PIDIV2);
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::FLOOR);
		//auto col = colliderobj.Get();
		//colliderobj.Init();
		//col->ShowDebug(true);
	}
}
