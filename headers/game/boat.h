#pragma once
#include "gameObjects/meshObject.h"
#include "DirectXMath.h"

class Boat : public MeshObject {
public:
	void Start() override;
	void Tick() override;
	void PhysicsTick() override;
	Boat() = default;
	void SetRotationPoint(DirectX::XMFLOAT3 point);
	void SetRadius(float radius);


	float speed = 20;
	float GetRadius() const;
	DirectX::XMFLOAT3 GetRotationPoint() const;


	float landingTime = 5;
	float time = landingTime;
	bool colliding = false;

private:
	DirectX::XMFLOAT3 rotationPoint{};
	float radius = 100;
};