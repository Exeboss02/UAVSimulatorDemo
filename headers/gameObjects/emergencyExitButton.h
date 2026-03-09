#pragma once
#include "gameObjects/meshObject.h"
#include "core/physics/sphereCollider.h"



class EmergenceExitButton : public MeshObject {
public:
	enum State {
		PreTouchUp,
		PostTouchUp
	};

	void Start() override;

	void Hover();

	void SetState(State state);

private:
	State state;
	std::weak_ptr<MeshObject> sign;
	std::weak_ptr<SphereCollider> collider;
	void OnInteract(std::shared_ptr<Player> player);
};