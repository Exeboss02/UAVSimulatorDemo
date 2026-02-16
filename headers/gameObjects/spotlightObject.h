#pragma once

#include "gameObjects/gameObject3D.h"
#include "rendering/renderQueue.h"
#include <DirectXMath.h>

class SpotlightObject : public GameObject3D {
public:
	struct SpotLightContainer {
		DirectX::XMFLOAT3 position;	 // 12 b
		DirectX::XMFLOAT3 direction; // 12 b

		DirectX::XMFLOAT4 color; // 16 b
		float intensity;		 // 4 b

		float spotAngleRadians; // 4 b
	};

	SpotlightObject();
	~SpotlightObject() = default;

	SpotLightContainer data;

	virtual void Start() override;
	virtual void Tick() override;

	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

private:
};