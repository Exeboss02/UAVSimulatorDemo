#pragma once
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/physics/boxCollider.h"
#include <vector>
#include <array>
#include <memory>

class Wall : public MeshObject {
public:
	virtual void OnObserve();
	virtual void OnInteract();

	virtual void Start() override;

	void SetWAllIndex(int wallIndex);

	void SetWallState(int wallState);

private:
	inline static const std::array<std::string, 3> wallMeshIdentifiers{"SpaceShip/room2.glb:Mesh_3", "SpaceShip/room2.glb:Mesh_1",
									   "SpaceShip/room2.glb:Mesh_2"};

	std::weak_ptr<BoxCollider> interactable;

	std::vector<std::weak_ptr<BoxCollider>> wallColliders;

	int wallIndex;

	void SpawnInteractables();

	void SpawnWallColliders(int wallStateIndex);

	void RemoveInteractables();
};
