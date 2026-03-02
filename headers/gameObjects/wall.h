#pragma once
#include "core/physics/boxCollider.h"
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "game/costHandler.h"

#include <array>
#include <memory>
#include <vector>

class Wall : public MeshObject {
public:
	virtual void OnObserve();
	virtual void OnInteract();

	void Interact(std::shared_ptr<Player> playerShared);

	virtual void Start() override;

	void SetWAllIndex(int wallIndex);

	void SetWallState(int wallState, bool edgeWall = false);

	void SetWallCost(size_t titanium, size_t lubricant, size_t carbonfiber, size_t circuitboards);

	void Hover();

private:
	inline static const std::array<std::string, 3> wallMeshIdentifiers{
		"SpaceShip/room2.glb:Mesh_3", "SpaceShip/room2.glb:Mesh_1", "SpaceShip/room2.glb:Mesh_2"};

	std::weak_ptr<BoxCollider> interactable;

	float hoverDisabledUntil = 0.0f;

	std::vector<std::weak_ptr<BoxCollider>> wallColliders;

	int wallIndex;

	void SpawnInteractables();

	void SpawnWallColliders(int wallStateIndex);

	void RemoveInteractables();

	CostHandler wallCost = CostHandler(2, 0, 0, 0);
};
