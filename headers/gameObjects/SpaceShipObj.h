#pragma once

// Add this forward declaration before any use of Room
class Room;

#include "gameObjects/gameObject3D.h"
#include "gameObjects/room.h"
#include "utilities/aStar.h"

#include <functional>
#include <memory>


class Cockpit;

struct Vector2Int {
	int x;
	int y;

	bool operator==(const Vector2Int& other) const { return x == other.x && y == other.y; }
};

namespace std {
template <>
struct hash<Vector2Int> {
	size_t operator()(const Vector2Int& v) const { return hash<int>()(v.x) ^ (hash<int>()(v.y) << 1); }
};
} // namespace std


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

	const std::unordered_map<Vector2Int, float>& GetPlacedRooms();

	/// <summary>
	/// Only overloaded for Imgui currently
	/// </summary>
	void Tick() override;

	void Start() override;

	std::unique_ptr<AStar>& GetPathfinder() { return this->pathfinder; }

	
	
	// These have to be public so other scripts (such as QuadTree) can use them for checks

	inline static const float ROOM_SIZE = 10.0f;
	static const size_t SHIP_MAX_SIZE_X = 15;
	static const size_t SHIP_MAX_SIZE_Y = 15;
	static const size_t START_ROOM_X = 7;
	static const size_t START_ROOM_Y = 0;

private:
	std::array<std::array<std::weak_ptr<Room>, SHIP_MAX_SIZE_Y>, SHIP_MAX_SIZE_X> rooms{};
	std::weak_ptr<Cockpit> cockpit;
	std::unordered_map<Vector2Int, float> placedRooms;

	std::unique_ptr<AStar> pathfinder;
	void CreateFloorColider();
};