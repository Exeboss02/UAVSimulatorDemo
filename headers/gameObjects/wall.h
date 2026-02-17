#pragma once
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/physics/boxCollider.h"
#include <vector>


class Wall : public MeshObject {
public:
	virtual void OnObserve() override;
	virtual void OnInteract() override;

	virtual void Start() override;

	void SetWAllIndex(int wallIndex);

	void SetWallState(int wallState);

private:
	std::string wallMeshIdentifiers[3]{"SpaceShip/room.glb:Mesh_3", "SpaceShip/room.glb:Mesh_1",
									   "SpaceShip/room.glb:Mesh_2"};

	std::weak_ptr<BoxCollider> interactable;

	std::vector<std::weak_ptr<BoxCollider>> wallColliders;

	int wallIndex;

	void SpawnInteractables();

	void SpawnWallColliders(int wallStateIndex);

	void RemoveInteractables();
};
