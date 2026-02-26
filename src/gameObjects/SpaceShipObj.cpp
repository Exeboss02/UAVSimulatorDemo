#include "gameObjects/SpaceShipObj.h"
#include "core/assetManager.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/testEnemy.h"
#include "gameObjects/enemy.h"
#include "imgui.h"
#include <array>

#include "utilities/logger.h"


SpaceShip::SpaceShip() : GameObject3D() { 
	Room::SetSize(this->ROOM_SIZE);
	pathfinder = std::make_unique<AStar>();
}

void SpaceShip::CreateRoom(size_t x, size_t y) {
	if (x < SHIP_MAX_SIZE_X && y < SHIP_MAX_SIZE_Y && rooms[x][y].expired()) {

		auto roomMesh = this->factory->CreateStaticGameObject<Room>();

		roomMesh->transform.SetPosition(DirectX::XMVectorSet(x * this->ROOM_SIZE, 0, y * this->ROOM_SIZE, 0));

		roomMesh->SetParent(this->GetPtr());

		roomMesh->SetPosition(x, y);

		roomMesh->SetupPathfindingNodes(std::dynamic_pointer_cast<SpaceShip>(this->GetPtr()), roomMesh);

		this->rooms[x][y] = roomMesh;

		// Calculations to determine which room is furthest from the cockpit
		int xFromStart = x - this->START_ROOM_X;
		this->placedRooms.emplace(Vector2Int{(int) x, (int) y}, static_cast<float>(xFromStart * xFromStart + y * y));
		
		roomMesh.Init();

		for (size_t i = 0; i < 4; i++) {
			Room::WallIndex wallIndex = (Room::WallIndex) i;
			std::array<int, 2> neighborOffset = Room::GetNeighborOffset(wallIndex);
			auto neighborWeak = this->GetRoom(x + neighborOffset[0], y + neighborOffset[1]);

			if (!neighborWeak.expired()) {
				auto neighbor = neighborWeak.lock();

				Room::WallIndex reversedWallIndex = (Room::WallIndex) ((i + 2) % 4);
				roomMesh->SetWallState(wallIndex, Room::WallState::door);
				neighbor->SetWallState(reversedWallIndex, Room::WallState::door);

				// Connect pathfinding nodes between the two rooms
				size_t currentRoomNodeIndex = wallIndex * 2 + 1;
				size_t neighborRoomNodeIndex = reversedWallIndex * 2 + 1;

				auto& currentRoomNodes = roomMesh->GetPathfindingNodes();
				auto& neighborRoomNodes = neighbor->GetPathfindingNodes();

				unsigned int edgeCost = static_cast<unsigned int>(this->ROOM_SIZE / 4);
				this->pathfinder->AddEdge(currentRoomNodes[currentRoomNodeIndex], neighborRoomNodes[neighborRoomNodeIndex], edgeCost);
			}
		}

		Logger::Log("Created Room");
	}
}

std::weak_ptr<Room> SpaceShip::GetRoom(size_t x, size_t y) {
	if (x < SHIP_MAX_SIZE_X && y < SHIP_MAX_SIZE_Y) return this->rooms[x][y];
	return std::weak_ptr<Room>();
}

const std::unordered_map<Vector2Int, float>& SpaceShip::GetPlacedRooms() { return this->placedRooms; }

void SpaceShip::Tick() {

	this->GameObject3D::Tick();
	static int pos[2]{};

	ImGui::Begin("Rooms");
	ImGui::InputInt2("cords", pos);
	bool roomCreator = ImGui::Button("Create Room");
	ImGui::End();

	if (roomCreator) {
		this->CreateRoom(pos[0], pos[1]);
	}
}

void SpaceShip::Start() {
	this->GameObject3D::Start();
	this->CreateFloorColider();
	
	// Create cockpit first to set the pathfinding goal
	auto cockpit = this->factory->CreateStaticGameObject<Cockpit>();
	cockpit->transform.SetPosition(SpaceShip::ROOM_SIZE * (SpaceShip::SHIP_MAX_SIZE_X / 2), 0,
								   -SpaceShip::ROOM_SIZE);
	cockpit->SetParent(this->GetPtr());
	cockpit->SetupPathfindingNodes(std::dynamic_pointer_cast<SpaceShip>(this->GetPtr()));

	// Now create the room - FindPath will work correctly
	CreateRoom(this->START_ROOM_X, this->START_ROOM_Y);
	auto room = this->GetRoom(this->START_ROOM_X, this->START_ROOM_Y);
	auto nodes = room.lock()->GetPathfindingNodes();

	// Connect cockpit to room
	room.lock()->SetWallState(Room::WallIndex::South, Room::WallState::door);
	this->pathfinder->AddEdge(nodes[Room::WallIndex::South * 2 + 1], cockpit->GetPathfindingNodes()[1], 1);
}

void SpaceShip::CreateFloorColider() {
	auto colliderobjWeak = this->factory->CreateGameObjectOfType<BoxCollider>();

	auto colliderobj = colliderobjWeak.lock();
	DirectX::XMFLOAT3 pos((this->SHIP_MAX_SIZE_X * this->ROOM_SIZE) * 0.5f, 0,
						  (this->SHIP_MAX_SIZE_Y * this->ROOM_SIZE) * 0.5f);
	colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	DirectX::XMFLOAT3 scale(((this->SHIP_MAX_SIZE_X) * this->ROOM_SIZE + this->ROOM_SIZE) * 0.5f, 0.5f,
							((this->SHIP_MAX_SIZE_Y) * this->ROOM_SIZE + this->ROOM_SIZE) * 0.5f);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	colliderobj->SetParent(this->GetPtr());
	colliderobj->SetTag(Tag::FLOOR);
	//colliderobj->ignoreTag = Tag::DISTANCE;

}