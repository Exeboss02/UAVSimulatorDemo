#pragma once
#include "gameObjects/meshObject.h"
#include "core/physics/sphereCollider.h"
#include "game/player.h"

class ResourceGenerator : public MeshObject {
public:
	void Start() override;

private:
	Interact(std::shared_ptr<Player> player);
	float lastGenerated = 0;

	float generatedPerSecond = 1;

	std::weak_ptr<SphereCollider> interactCollider;
};