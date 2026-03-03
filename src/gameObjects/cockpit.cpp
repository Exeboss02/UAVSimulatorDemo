#include "gameObjects/cockpit.h"
#include "core/physics/sphereCollider.h"
#include "game/gameManager.h"
#include "game/gunPickUp.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/emergencyExitButton.h"
#include "gameObjects/room.h"
#include "utilities/aStar.h"
#include <numbers>

void Cockpit::Start() {
	this->SetName("Cockpit");
	auto coreCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	coreCollider->SetParent(this->GetPtr());
	coreCollider->SetSolid(true);
	coreCollider->SetTag(Tag::PLAYER);

	auto coreMesh = this->factory->CreateStaticGameObject<MeshObject>();
	coreMesh->SetParent(this->GetPtr());
	coreMesh->transform.SetPosition(0, 2, 0);
	coreMesh->transform.SetScale(1, 1.2f, 1);
	coreMesh->SetMesh(AssetManager::GetInstance().GetMeshObjData("meshes/core.glb:Mesh_0"));

	// Update core health on hud
	GameManager::GetInstance()->GetPlayer()->hud->SetCoreHealthText(this->health.Get());

	coreCollider->SetOnHit([&](float damage) {
		this->health.Decrement(damage);
		if (this->health.IsDead()) {
			this->onDeath();
			Logger::Log("Core Died");

			// create tempspeaker
			// SoundClip* deathClip = AssetManager::GetInstance().GetSoundClip("BigExplosion2.wav");
			// std::weak_ptr<SoundSourceObject> deathSpeaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
			// deathSpeaker.lock()->SetDeleteWhenFinnished(true);
			// deathSpeaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
			// deathSpeaker.lock()->Play(deathClip);
			// GameManager::GetInstance()->Loose();
		}

		// Update core health on hud
		GameManager::GetInstance()->GetPlayer()->hud->SetCoreHealthText(this->health.Get());

		// SoundClip* damageClip = AssetManager::GetInstance().GetSoundClip("SmallExplotion.wav");
		// this->speaker.lock()->SetDeleteWhenFinnished(true);
		// this->speaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
		// this->speaker.lock()->Play(damageClip);
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
	emergencyButton->transform.SetPosition(4.43, 2, 0);
	emergencyButton->SetParent(this->GetPtr());

	auto gunPickUp = this->factory->CreateStaticGameObject<GunPickUp>();
	gunPickUp->transform.SetPosition(-4, 0.58, 4);
	gunPickUp->transform.SetRotationRPY(0, 3.14 / 2, 3.14 * 1.75);

	auto globalScale = this->transform.GetGlobalScale();

	// Scaling down from spaceship scaling
	gunPickUp->transform.SetScale(1 / globalScale.m128_f32[0], 1 / globalScale.m128_f32[1],
								  1 / globalScale.m128_f32[2]);

	gunPickUp->SetParent(this->GetPtr());


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

	{
		auto meshobj = this->factory->CreateStaticGameObject<MeshObject>();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room2.glb:Mesh_0");

		meshobj->SetMesh(meshdata);
	}
	{
		auto meshobj = this->factory->CreateStaticGameObject<MeshObject>();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room2.glb:Mesh_4");
		meshobj->SetMesh(meshdata);
	}

	for (size_t i = 0; i < 4; i++) {

		auto meshobj = this->factory->CreateStaticGameObject<Wall>();

		meshobj->SetParent(this->GetPtr());

		meshobj->transform.SetRotationRPY(0, 0, i * std::numbers::pi / 2);

		meshobj->SetWallState(Room::WallState::window, true);

		meshobj->SetWAllIndex(i);

		this->walls[i] = meshobj;
	}

	this->walls[0].lock()->SetWallState(Room::WallState::door);
	this->walls[1].lock()->SetWallState(Room::WallState::solid, true);
	this->walls[3].lock()->SetWallState(Room::WallState::solid, true);
}
