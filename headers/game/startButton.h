#pragma once
#include "gameObjects/meshObject.h"
#include "core/physics/sphereCollider.h"



class StartButton : public MeshObject {
public:
	void Start() override;

private:
	std::weak_ptr<MeshObject> sign;
	std::weak_ptr<SphereCollider> collider;
	void OnInteract(std::shared_ptr<Player> player);
};