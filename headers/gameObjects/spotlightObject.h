#pragma once

#include "gameObjects/gameObject3D.h"
#include "rendering/renderQueue.h"
#include <DirectXMath.h>
#include "rendering/depthBuffer.h"
#include "gameObjects/cameraObject.h"

class SpotlightObject : public GameObject3D {
public:
	struct alignas(16) SpotLightContainer {
		DirectX::XMFLOAT3 position;	 // 12 b
		DirectX::XMFLOAT3 direction; // 12 b

		DirectX::XMFLOAT4 color; // 16 b
		float intensity;		 // 4 b

		float spotCosAngleRadians; // 4 b

		DirectX::XMFLOAT4X4 viewProjectionMatrix;
	};



	SpotlightObject();
	~SpotlightObject() = default;


	SpotLightContainer GetSpotLightData() const;

	/// <summary>
	/// Return the spotlight angle in degrees
	/// </summary>
	/// <returns></returns>
	float GetAngle() const;

	virtual void Start() override;
	virtual void Tick() override;

	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	std::weak_ptr<CameraObject> camera;

	/// <summary>
	/// Sets the light angle in degrees
	/// </summary>
	/// <param name="angle"></param>
	void SetAngle(float angle);

	void SetIntensity(float intensity);

	virtual void ShowInHierarchy() override;

	virtual void OnDestroy() override;

private:

	SpotLightContainer data;
};