#include "gameObjects/room.h"
#include "core/assetManager.h"
#include "gameObjects/spaceShipObj.h"
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
		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room.glb:Mesh_0");
		meshobj->SetMesh(meshdata);

		this->roof = meshobj;
	}
	{
		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room.glb:Mesh_4");
		meshobj->SetMesh(meshdata);

		this->floor = meshobj;
	}
	for (size_t i = 0; i < 4; i++) {

		auto meshobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();

		auto meshobj = meshobjWeak.lock();
		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData(
			this->wallMeshIdentifiers[static_cast<int>(Room::WallState::window)]);
		meshobj->SetMesh(meshdata);

		meshobj->transform.SetRotationRPY(0, 0, i * std::numbers::pi / 2);
		this->floor = meshobj;

		this->walls[i] = meshobj;
	}
}

void Room::Tick() { this->GameObject3D::Tick(); }

void Room::SetSize(float size) { Room::size = size; }

void Room::SetWallState(Room::WallIndex wallindex, Room::WallState wallState) {

	std::shared_ptr<MeshObject> wall = this->walls[wallindex].lock();
	wall->SetMesh(AssetManager::GetInstance().GetMeshObjData(this->wallMeshIdentifiers[static_cast<int>(wallState)]));
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
