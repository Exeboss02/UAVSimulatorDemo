#include "gameObjects/cockpit.h"
#include "core/physics/sphereCollider.h"
#include "gameObjects/room.h"
#include "game/gameManager.h"

void Cockpit::Start() { 
	auto coreCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	coreCollider->SetParent(this->GetPtr());
	coreCollider->SetSolid(true);

	// Update core health on hud
	GameManager::GetInstance()->GetPlayer()->hud->SetCoreHealthText(this->health.Get());

	coreCollider->SetOnHit([&](float damage) { 
		this->health.Decrement(damage);
		if (this->health.IsDead()) {
			this->onDeath();
			Logger::Log("Core Died");
		}

		// Update core health on hud
		GameManager::GetInstance()->GetPlayer()->hud->SetCoreHealthText(this->health.Get());
	});
	coreCollider->transform.SetPosition(0, 2, 0);
	coreCollider->transform.SetScale(2, 2, 2);

	this->corePosition = coreCollider->transform.GetGlobalPosition();

	//this->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	//auto tempCockpitMesh = this->factory->CreateStaticGameObject<MeshObject>();
	//tempCockpitMesh->SetParent(this->GetPtr());
	//tempCockpitMesh->transform.SetPosition(0, 3, 0);
	//tempCockpitMesh->transform.SetScale(5, 2.5, 5);
	//tempCockpitMesh->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));

	this->GameObject3D::Start(); 
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
