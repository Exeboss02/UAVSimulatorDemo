#pragma once

#include "gameObjects/cameraObject.h"
#include "gameObjects/gameObject3D.h"
#include "rendering/shadowCube.h"
#include "rendering/renderQueue.h"
#include <DirectXMath.h>

class PointLightObject : public GameObject3D {
public:
	struct alignas(16) PointLightContainer {
		DirectX::XMFLOAT3 position;  // 12 b

		DirectX::XMFLOAT4 color; // 16 b
		float intensity;		 // 4 b

		DirectX::XMFLOAT4X4 viewProjectionMatrix[6];
	};

	PointLightObject();
	~PointLightObject() = default;

	PointLightContainer GetPointLightData();

	virtual void Start() override;
	virtual void Tick() override;

	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	std::array<std::weak_ptr<CameraObject>, 6> cameras;

	virtual void ShowInHierarchy() override;

private:

	PointLightContainer data;
};