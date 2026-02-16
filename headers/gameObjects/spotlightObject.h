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
	ID3D11DepthStencilView* GetDepthStencilView() const;
	const D3D11_VIEWPORT& GetViewPort() const;
	bool GetResolutionChanged() const;
	ID3D11ShaderResourceView* GetSRV() const;

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
	void SetShadowResolution(size_t width, size_t height);
	void SetDepthBuffer(ID3D11Device* device);

	/// <summary>
	/// Sets the light angle in degrees
	/// </summary>
	/// <param name="angle"></param>
	void SetAngle(float angle);


	virtual void ShowInHierarchy() override;

private:

	bool resolutionChanged = true;

	D3D11_VIEWPORT shadowViewPort;
	SpotLightContainer data;
	DepthBuffer shadowbuffer;
};