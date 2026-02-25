#pragma once
#include "gameObjects/meshObject.h"
#include "core/physics/sphereCollider.h"

class Mine : public MeshObject {
public:
    void Start() override;
private:
	void OnExplode();



    std::weak_ptr<SphereCollider> collider;
};