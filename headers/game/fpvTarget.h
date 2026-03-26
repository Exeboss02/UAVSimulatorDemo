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
	void Tick() override;
	void OnExplode();



private:

	Vector3D Lerp(const Vector3D& start, const Vector3D& end, float val);
	Vector3D startPos = Vector3D(200, 200, 0);
	Vector3D endPos = Vector3D(0, 0, 0);
	float timer = 0.0f;
	float hitTime = 20.0f;

	void move();

	std::string gunVisualPath = "drones/geranium-2.glb:Mesh_0";


	std::weak_ptr<MeshObject> targetMesh;

	std::weak_ptr<BoxCollider> targetCollider;


};
