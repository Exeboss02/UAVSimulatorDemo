#pragma once

// Add this forward declaration before any use of Room
class Room;

#include "gameObjects/gameObject3D.h"
#include "gameObjects/room.h"
#include "utilities/aStar.h"

class SpaceShip : public GameObject3D {
public:
	SpaceShip();

	/// <summary>
	/// Creates a room and makes sure the room isn't beyond max size,
	/// Note that this doesn't check if there is any way to get to the room.
	/// It is recommended to call create room from a room to ensure it is valid
	/// for the player to create said room
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	void CreateRoom(size_t x, size_t y);

	/// <summary>
	/// returns a reference to a room
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
	std::weak_ptr<Room> GetRoom(size_t x, size_t y);

	/// <summary>
	/// Only overloaded for Imgui currently
	/// </summary>
	void Tick() override;

	void Start() override;

	std::unique_ptr<AStar>& GetPathfinder() { return this->pathfinder; }

private:
	inline static const float ROOM_SIZE = 10.0f;
	static const size_t SHIP_MAX_SIZE_X = 63;
	static const size_t SHIP_MAX_SIZE_Y = 63;
	std::array<std::array<std::weak_ptr<Room>, SHIP_MAX_SIZE_Y>, SHIP_MAX_SIZE_X> rooms{};
	
	struct Vector2Int {
		int x;
		int y;
	};
	std::unordered_map<Vector2Int, float> placedRooms;

	std::unique_ptr<AStar> pathfinder;
	std::vector<std::shared_ptr<AStarVertex>> path;
	void CreateFloorColider();
};