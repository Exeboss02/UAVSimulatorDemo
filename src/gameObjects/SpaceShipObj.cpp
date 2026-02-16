#include "gameObjects/SpaceShipObj.h"
#include "core/assetManager.h"
#include "gameObjects/meshObject.h"
#include "imgui.h"
#include <array>

SpaceShip::SpaceShip() : GameObject3D() { Room::SetSize(this->ROOM_SIZE); }

void SpaceShip::CreateRoom(size_t x, size_t y) {
	if (x < SHIP_MAX_SIZE_X && y < SHIP_MAX_SIZE_Y && rooms[x][y].expired()) {

		std::weak_ptr<Room> room = this->factory->CreateGameObjectOfType<Room>();

		if (room.expired()) {
			Logger::Error("What just happend??");
		}

		auto roomMesh = room.lock();

		roomMesh->transform.SetPosition(DirectX::XMVectorSet(x * ROOM_SIZE, 0, y * ROOM_SIZE, 0));

		roomMesh->SetParent(this->GetPtr());

		roomMesh->SetPosition(x, y);

		this->rooms[x][y] = room;

		for (size_t i = 0; i < 4; i++) {
			Room::WallIndex wallIndex = (Room::WallIndex) i;
			std::array<int, 2> neighborOffset = Room::GetNeighborOffset(wallIndex);
			auto neighborWeak = this->GetRoom(x + neighborOffset[0], y + neighborOffset[1]);

			if (!neighborWeak.expired()) {
				auto neighbor = neighborWeak.lock();

				Room::WallIndex reversedWallIndex = (Room::WallIndex) ((i + 2) % 4);
				roomMesh->SetWallState(wallIndex, Room::WallState::door);
				neighbor->SetWallState(reversedWallIndex, Room::WallState::door);
			}
		}

		Logger::Log("Created Room");
	}
}

std::weak_ptr<Room> SpaceShip::GetRoom(size_t x, size_t y) {
	if (x < SHIP_MAX_SIZE_X && y < SHIP_MAX_SIZE_Y) return this->rooms[x][y];
	return std::weak_ptr<Room>();
}

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