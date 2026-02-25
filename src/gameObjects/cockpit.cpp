#include "gameObjects/cockpit.h"
#include "core/physics/sphereCollider.h"
#include "gameObjects/room.h"

void Cockpit::Start() { 
	auto coreCollider = this->factory->CreateStaticGameObject<SphereCollider>();
	coreCollider->SetParent(this->GetPtr());
	coreCollider->SetSolid(true);

	coreCollider->SetOnHit([&](float damage) { 
		this->health -= damage;
		if (this->health < 0) {
			this->onDeath();
			Logger::Log("Core Died");
		}
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

void Cockpit::SetupPathfindingNodes(std::shared_ptr<SpaceShip> spaceShip, std::shared_ptr<Cockpit> cockpitPtr) {
	
	size_t nodeCount = this->pathfindingNodes.size();

	// Core node
	auto vertex = this->factory->CreateGameObjectOfType<AStarVertex>().lock();
	vertex->Initialize(this->corePosition);
	vertex->transform.SetPosition(this->corePosition);

	vertex->SetParent(cockpitPtr);

	this->pathfindingNodes[0] = vertex;
	spaceShip->GetPathfinder()->AddVertex(vertex);

	// Door node
	auto doorVertex = this->factory->CreateGameObjectOfType<AStarVertex>().lock();
	DirectX::XMVECTOR doorPosition =
		DirectX::XMVectorAdd(this->pathfindingNodes[0]->transform.GetPosition(), DirectX::XMVectorSet(0, 2, 3, 0));
	doorVertex->Initialize(DirectX::XMVectorAdd(this->corePosition, doorPosition));
	doorVertex->transform.SetPosition(doorPosition);

	doorVertex->SetParent(cockpitPtr);

	this->pathfindingNodes[1] = doorVertex;
	spaceShip->GetPathfinder()->AddVertex(doorVertex);

	// Edge
	spaceShip->GetPathfinder()->AddEdge(this->pathfindingNodes[0], this->pathfindingNodes[1], 1);

	// Set pathfinding goal to core
	spaceShip->GetPathfinder()->SetGoal(this->pathfindingNodes[0]);
}
