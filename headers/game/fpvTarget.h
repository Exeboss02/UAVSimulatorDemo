#pragma once
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/physics/collision.h"
#include "core/physics/vector3D.h"

class fpvTarget : public GameObject3D {
public:
	fpvTarget();
	~fpvTarget();

	void Start() override;
	void OnExplode();



private:

	std::string gunVisualPath = "drones/geranium-2.glb:Mesh_0";


	std::weak_ptr<MeshObject> targetMesh;

	std::weak_ptr<BoxCollider> targetCollider;


};
