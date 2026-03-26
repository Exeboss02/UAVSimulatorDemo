#pragma once
#include "gameObjects/meshObject.h"
#include "DirectXMath.h"

class Boat : MeshObject {
public:
	void Start() override;
	void Tick() override;
	Boat() = default;
	void SetRotationPoint(DirectX::XMFLOAT3 point);
	void SetRadius(float radius);


	float speed = 1;
	float GetRadius() const;
	DirectX::XMFLOAT3 GetRotationPoint() const;

private:
	DirectX::XMFLOAT3 rotationPoint{};
	float radius;
};