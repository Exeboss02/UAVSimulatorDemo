#pragma once
#include "core/physics/physicsQueue.h"
#include "core/tools.h"
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "game/player.h"

class GunPickUp : public MeshObject {
public:
	GunPickUp();
	~GunPickUp();

	void Start() override;

	void OnInteract(std::shared_ptr<Player> playerShared);

	void Hover();


private:

		std::weak_ptr<BoxCollider> interactable;



};
