#pragma once
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"


class Room : public GameObject3D {
public:
	enum WallIndex {
		North,
		East,
		South, 
		West,
	};
	enum WallState {
		solid,
		window,
		door
	};
	std::string wallMeshIdentifiers[3]{"SpaceShip/room.glb:Mesh_3", "SpaceShip/room.glb:Mesh_1", 
		"SpaceShip/room.glb:Mesh_2"};

	/// <summary>
	/// Helper function that returns a index offset for the selected neighbor.
	/// For example {0, 1} for North
	/// </summary>
	/// <param name="wallIndex"></param>
	/// <returns></returns>
	static std::array<int, 2> GetNeighborOffset(WallIndex wallIndex);

	/// <summary>
	/// Sets the size of all rooms, should generally not be touched except by SpaceShip at 
	/// SpaceShip construction
	/// </summary>
	/// <param name="size"></param>
	static void SetSize(float size);

	/// <summary>
	/// Overrided to create all it's walls / floor / roof
	/// </summary>
	void Start() override;

	void Tick() override;

	/// <summary>
	/// Creates a room in the specified direction, 
	/// </summary>
	/// <param name="wallIndex"></param>
	void CreateRoom(WallIndex wallIndex);

	/// <summary>
	/// Sets the array position, should only be set by SpaceShip at room creation.
	/// Used to allow rooms to know who their neighbors are.
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	void SetPosition(size_t x, size_t y);

	/// <summary>
	/// Removes or replaces walls
	/// </summary>
	/// <param name="wall"></param>
	/// <param name="active"></param>
	void SetWallState(Room::WallIndex wall, Room::WallState wallState);

	/// <summary>
	/// Overridden to dissallow any parent other than SpaceShip
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(std::weak_ptr<GameObject> parent) override;

private:


	std::array<size_t, 2> pos;
	inline static float size;
	std::weak_ptr<MeshObject> roof;
	std::weak_ptr<MeshObject> floor;
	std::array<std::weak_ptr<MeshObject>, 4> walls;
};