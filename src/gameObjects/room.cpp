#include "gameObjects/room.h"
#include "core/assetManager.h"

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

	this->SetupPathfindingNodes(parent);

	auto neighOffset = Room::GetNeighborOffset(wallIndex);
	Logger::Log("Position: ", this->pos[0], " ", this->pos[1]);
	parent->CreateRoom(this->pos[0] + neighOffset[0], this->pos[1] + neighOffset[1]);
}

void Room::SetPosition(size_t x, size_t y) { this->pos = {x, y}; }

void Room::Start() {
	Logger::Warn("room size ", this->size);
	{
		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		meshobj->SetMesh(meshdata);

		meshobj->transform.SetPosition(DirectX::XMVectorSet(0, 5, 0, 0));
		meshobj->transform.SetScale(DirectX::XMVectorSet(this->size, 1, this->size, 1));

		this->roof = meshobj;
	}
	{
		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		meshobj->SetMesh(meshdata);

		meshobj->transform.SetPosition(DirectX::XMVectorSet(0, -5, 0, 0));
		meshobj->transform.SetScale(DirectX::XMVectorSet(this->size, 1, this->size, 1));

		this->floor = meshobj;
	}
	for (size_t i = 0; i < 4; i++) {

		std::array<float, 2> wallpos = {static_cast<float>(wallpositions[i][0]), static_cast<float>(wallpositions[i][1])};

		wallpos[0] *= this->size / 2;
		wallpos[1] *= this->size / 2;

		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		meshobj->SetMesh(meshdata);

		meshobj->transform.SetPosition(DirectX::XMVectorSet(wallpos[0], 0, wallpos[1], 0));

		if (wallpos[0] == 0) {
			meshobj->transform.SetScale(DirectX::XMVectorSet(this->size, 5, 1, 1));
		} else {
			meshobj->transform.SetScale(DirectX::XMVectorSet(1, 5, this->size, 1));
		}

		this->walls[i] = meshobj;
	}
}

void Room::Tick() {
	this->GameObject3D::Tick();
}

void Room::SetSize(float size) { Room::size = size; }

void Room::SetWallState(Room::WallIndex wallindex, bool active) {

	auto& wall = this->walls[wallindex];

	if (active == false && !wall.expired()) {
		this->factory->QueueDeleteGameObject(wall);
	} else if (!active && wall.expired()) {
		std::array<float, 2> wallpos = {static_cast<float>(wallpositions[wallindex][0]), static_cast<float>(wallpositions[wallindex][1])};

		wallpos[0] *= this->size / 2;
		wallpos[1] *= this->size / 2;

		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
		meshobj->SetMesh(meshdata);

		meshobj->transform.SetPosition(DirectX::XMVectorSet(wallpos[0], 0, wallpos[1], 0));

		if (wallpos[0] == 0) {
			meshobj->transform.SetScale(DirectX::XMVectorSet(this->size, 5, 1, 1));
		} else {
			meshobj->transform.SetScale(DirectX::XMVectorSet(1, 5, this->size, 1));
		}

		this->walls[wallindex] = meshobj;
	}
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

void Room::SetupPathfindingNodes(std::shared_ptr<SpaceShip> parent) { 
	// 9 nodes in local room-space, 1 in the center, and 8 around the center in a circle, with radius size / 4, starting
	// with the one directly east of the center and going clockwise
	std::array<DirectX::XMVECTOR, 9> nodesLocal;
	nodesLocal[0] = DirectX::XMVectorSet(0, 0, 0, 1);
	for (size_t i = 0; i < 8; i++) {
		float angle = i * (DirectX::XM_PI / 4);
		nodesLocal[i] = DirectX::XMVectorSet(std::cos(angle) * (this->size / 4), 0, std::sin(angle) * (this->size / 4), 1);
	}

	// Create vertices for each node, translate to world space
	for (size_t i = 0; i < this->pathfindingNodes.size(); ++i) {
		this->pathfindingNodes[i]->SetParent(this->GetPtr());
		this->pathfindingNodes[i] = std::make_shared<AStarVertex>(nodesLocal[i]);

		parent->GetPathfinder()->AddVertex(this->pathfindingNodes[i]);
	}

	// Create edges from all outside nodes to the center node
	for (size_t i = 1; i < this->pathfindingNodes.size(); ++i) {
		parent->GetPathfinder()->AddEdge(this->pathfindingNodes[0], this->pathfindingNodes[i],
										 static_cast<unsigned int>(this->size / 3));
	}

	// Create edges between adjacent outside nodes
	for (size_t i = 1; i < this->pathfindingNodes.size(); ++i) {
		size_t nextIndex = (i % 8) + 1;
		parent->GetPathfinder()->AddEdge(this->pathfindingNodes[i], this->pathfindingNodes[nextIndex],
										 static_cast<unsigned int>(this->size / 4));
	}
}
