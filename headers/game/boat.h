#pragma once
#include "gameObjects/meshObject.h"
#include "DirectXMath.h"

class Boat : public MeshObject {
public:
	void Start() override;
	void Tick() override;
	Boat() = default;
	void SetRotationPoint(DirectX::XMFLOAT3 point);
	void SetRadius(float radius);


	float speed = 20;
	float GetRadius() const;
	DirectX::XMFLOAT3 GetRotationPoint() const;

private:
	DirectX::XMFLOAT3 rotationPoint{};
	float radius = 100;
};