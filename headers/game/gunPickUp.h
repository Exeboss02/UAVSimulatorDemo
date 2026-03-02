#pragma once
#include "core/physics/physicsQueue.h"
#include "core/tools.h"
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "game/player.h"
#include "game/costHandler.h"

class GunPickUp : public MeshObject {
public:
	GunPickUp();
	~GunPickUp();

	void Start() override;

	void OnInteract(std::shared_ptr<Player> playerShared);

	void Hover();


private:
	std::string gunVisualPath = "guns/GunTest.glb:Mesh_0";
	DirectX::XMFLOAT3 visualPosition = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 visualScale = DirectX::XMFLOAT3(0.007f, 0.007f, 0.007f);
	DirectX::XMFLOAT3 visualRotationRPY = DirectX::XMFLOAT3(0, 0, 0);

	std::weak_ptr<BoxCollider> interactable;

	std::weak_ptr<MeshObject> gunVisual;

	float hoverDisabledUntil = 0.0f;

	CostHandler gunCost = CostHandler(100, 0, 0, 0);

};
