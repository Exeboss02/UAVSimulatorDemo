#include "gameObjects/room.h"
#include "core/assetManager.h"
#include "gameObjects/spaceShipObj.h"
#include "gameObjects/spotlightObject.h"
#include "gameObjects/turret.h"
#include <numbers>

static const std::array<std::array<int, 2>, 4> wallpositions = {
	std::array<int, 2>({0, 1}),
	std::array<int, 2>({1, 0}),
	std::array<int, 2>({0, -1}),
	std::array<int, 2>({-1, 0}),
};

std::array<int, 2> Room::GetNeighborOffset(Room::WallIndex wallIndex) { return wallpositions[wallIndex]; }

void Room::CreateRoom(WallIndex wallIndex) {
	Logger::Log("Creating room");
	auto parentWeak = this->GetParent();
	if (parentWeak.expired()) {
		Logger::Error("Room parent is dead, how in the funk?");
		return;
	}

	auto parent = std::static_pointer_cast<SpaceShip>(parentWeak.lock());

	auto neighOffset = Room::GetNeighborOffset(wallIndex);
	Logger::Log("Position: ", this->pos[0], " ", this->pos[1]);
	parent->CreateRoom(this->pos[0] + neighOffset[0], this->pos[1] + neighOffset[1]);
}

void Room::SetPosition(size_t x, size_t y) { this->pos = {x, y}; }

void Room::Start() {
	Logger::Warn("room size ", this->size);
	{
		auto meshobj = this->factory->CreateStaticGameObject<MeshObject>();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room2.glb:Mesh_0");

		meshobj->SetMesh(meshdata);

		this->roof = meshobj;
	}
	{
		auto meshobj = this->factory->CreateStaticGameObject<MeshObject>();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room2.glb:Mesh_4");
		meshobj->SetMesh(meshdata);

		this->floor = meshobj;
	}
	for (size_t i = 0; i < 4; i++) {

		auto meshobj = this->factory->CreateStaticGameObject<Wall>();

		meshobj->SetParent(this->GetPtr());

		meshobj->transform.SetRotationRPY(0, 0, i * std::numbers::pi / 2);

		meshobj->SetWallState(Room::WallState::window);

		meshobj->SetWAllIndex(i);

		meshobj->SetName("A WALL");

		this->walls[i] = meshobj;
	}
	auto buildCollider = this->factory->CreateStaticGameObject<BoxCollider>();
	buildCollider->transform.SetScale({1, .5, 1});
	buildCollider->transform.SetPosition({0, 1, 0});
	buildCollider->SetParent(this->GetPtr());
	buildCollider->solid = false;
	buildCollider->SetName("BUILD COLLIDER");
	// Maybe tweak positionW
	this->buildSlot = buildCollider;
	buildCollider->SetOnInteract([&]() {
		if (this->builtObject.expired()) {
			auto turret = this->factory->CreateStaticGameObject<Turret>();
			turret->SetParent(this->GetPtr());
			turret->transform.SetPosition({0, 1.5, 0});
			//turret->SetMesh(AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0"));
		}
		this->factory->QueueDeleteGameObject(this->buildSlot);
	});

	auto spotLight = this->factory->CreateGameObjectOfType<SpotlightObject>().lock();
	spotLight->SetParent(this->GetPtr());
	spotLight->transform.SetPosition({0, 4.5, 0});
	spotLight->transform.SetRotationRPY(0, std::numbers::pi / 2, 0);
	spotLight->SetAngle(120.);
	spotLight->SetIntensity(10.);
}

void Room::Tick() { this->GameObject3D::Tick(); }

void Room::SetSize(float size) { Room::size = size; }

void Room::SetWallState(Room::WallIndex wallindex, Room::WallState wallState) {

	std::shared_ptr<Wall> wall = this->walls[wallindex].lock();
	wall->SetWallState(wallState);
}

void Room::SetParent(std::weak_ptr<GameObject> parentWeak) {
	if (auto basePtr = parentWeak.lock()) {
		if (auto shipPtr = std::dynamic_pointer_cast<SpaceShip>(basePtr)) {
			this->GameObject3D::SetParent(parentWeak);
		} else {
			Logger::Error("Trying to set non SpaceShip obj as parent to Room");
			return;
		}
	} else {
		Logger::Error("Trying to set parent when parent doesn't exist???");
		return;
	}
}

void Room::SetupPathfindingNodes(std::shared_ptr<SpaceShip> spaceShip, std::shared_ptr<Room> roomPtr) {

	size_t nodeCount = this->pathfindingNodes.size();

	const float nodeHeight = 2.0f;

	// 9 nodes in local room-space, 1 in the center, and 8 around the center in a circle.
	std::array<DirectX::XMVECTOR, 9> nodesLocal = {};
	nodesLocal[0] = DirectX::XMVectorSet(0, nodeHeight, 0, 1);
	float distance = this->size / 4;
	for (size_t i = 1; i < nodeCount; ++i) {
		float angle = DirectX::XM_PIDIV2 - (i - 1) * (DirectX::XM_PI / 4); // Start at North (z+) and go clockwise
		nodesLocal[i] = DirectX::XMVectorSet(std::cos(angle) * distance, nodeHeight, std::sin(angle) * distance, 1);
	}

	// Create vertices for each node using the factory
	for (size_t i = 0; i < nodeCount; ++i) {
		// Create through factory so myPtr is initialized properly
		auto vertexWeak = this->factory->CreateGameObjectOfType<AStarVertex>();
		auto vertex = vertexWeak.lock();

		// Initialize the vertex with its position
		vertex->Initialize(nodesLocal[i]);
		vertex->transform.SetPosition(nodesLocal[i]);

		// Now SetParent will work because myPtr is initialized by the factory
		vertex->SetParent(roomPtr);

		this->pathfindingNodes[i] = vertex;

		spaceShip->GetPathfinder()->AddVertex(vertex);
	}

	// Create edges from all outside nodes to the center node
	unsigned int edgeCost = static_cast<unsigned int>(this->size / 3);
	for (size_t i = 1; i < nodeCount; ++i) {
		spaceShip->GetPathfinder()->AddEdge(this->pathfindingNodes[0], this->pathfindingNodes[i], edgeCost);
	}

	edgeCost = static_cast<unsigned int>(this->size / 4);
	// Create edges between adjacent outside nodes
	for (size_t i = 1; i < nodeCount; ++i) {
		size_t nextIndex = (i % 8) + 1;
		spaceShip->GetPathfinder()->AddEdge(this->pathfindingNodes[i], this->pathfindingNodes[nextIndex], edgeCost);
	}
}