#pragma once
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/physics/boxCollider.h"


class Wall : public MeshObject {
public:
	virtual void OnObserve() override;
	virtual void OnInteract() override;

	virtual void Start() override;

private:
	std::weak_ptr<BoxCollider> interactable;
};
