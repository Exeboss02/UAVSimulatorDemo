#include "gameObjects/pointLightObject.h"

PointLightObject::PointLightObject() {
	this->data = {};
	DirectX::XMStoreFloat3(&this->data.position, DirectX::XMVectorSet(0, 0, 0, 0));
	DirectX::XMStoreFloat4(&this->data.color, DirectX::XMVectorSet(1, 1, 1, 1));

	this->shadowViewPort = {
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<FLOAT>(128),
		.Height = static_cast<FLOAT>(128),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};

	Logger::Log("Created a pointLight.");
	this->resolutionChanged = true;
}

PointLightObject::PointLightContainer PointLightObject::GetPointLightData() {
	DirectX::XMStoreFloat3(&this->data.position, GetGlobalPosition());

	for (size_t i = 0; i < 6; i++) {
		if (this->cameras[i].expired()) {
			Logger::Error("One of PointLight shadowcameras were expired");
			continue;
		}
		this->data.viewProjectionMatrix[i] = this->cameras[i].lock()->GetCameraMatrix(true).viewProjectionMatrix;
	}

	return this->data;
}

std::array<ID3D11DepthStencilView*, 6> PointLightObject::GetDepthStencilViews() const {
	return {
		this->shadowCubeMap.GetDsv(0),
		this->shadowCubeMap.GetDsv(1),
		this->shadowCubeMap.GetDsv(2),
		this->shadowCubeMap.GetDsv(3),
		this->shadowCubeMap.GetDsv(4),
		this->shadowCubeMap.GetDsv(5),
	};
}

const D3D11_VIEWPORT& PointLightObject::GetViewPort() const {
	if (this->resolutionChanged) {
		Logger::Warn("Note that shadow resolution has changed But buffer has not"
					 ", Do not use this viewport for rendering shadow before calling SetDepthBuffer");
	}
	return this->shadowViewPort;
}

bool PointLightObject::GetResolutionChanged() const { return this->resolutionChanged; }

ID3D11ShaderResourceView* PointLightObject::GetSRV() const { return this->shadowCubeMap.GetSrv(); }

void PointLightObject::Start() {
	RenderQueue::AddPointLight(this->GetPtr());

	// Attach camera for shadowpass
	for (size_t i = 0; i < 6; i++) {
		this->cameras[i] = this->factory->CreateGameObjectOfType<CameraObject>();
		this->cameras[i].lock()->SetAspectRatio(1 / 1);
		this->cameras[i].lock()->SetFarPlane(20.);
		this->cameras[i].lock()->SetFov(90);
		this->cameras[i].lock()->SetParent(this->GetPtr());
	}

	SetCameraOrientation(this->cameras[0].lock().get(), {1,0,0}, {0,1,0});
	SetCameraOrientation(this->cameras[1].lock().get(), {-1,0,0}, {0,1,0});
	SetCameraOrientation(this->cameras[2].lock().get(), {0,1,0}, {0,0,-1});
	SetCameraOrientation(this->cameras[3].lock().get(), {0,-1,0}, {0,0,1});
	SetCameraOrientation(this->cameras[4].lock().get(), {0,0,1}, {0,1,0});
	SetCameraOrientation(this->cameras[5].lock().get(), {0,0,-1}, {0,1,0});
}

void PointLightObject::Tick() {

	// feels bad doing this every frame
	DirectX::XMStoreFloat3(&this->data.position, GetGlobalPosition());

	for (size_t i = 0; i < 6; i++) {
		auto& cameraWeak = this->cameras[i];
		if (cameraWeak.expired()) {
			Logger::Error("Pointligt camera was expired");
			continue;
		}
		auto cameraLocked = cameraWeak.lock();

		auto viewProj = cameraLocked->GetCameraMatrix(true).viewProjectionMatrix;

		this->data.viewProjectionMatrix[i] = viewProj;
	}
}

void PointLightObject::LoadFromJson(const nlohmann::json& data) {
	this->GameObject3D::LoadFromJson(data);

	if (data.contains("color")) {
		auto newColor = data["color"];
		DirectX::XMStoreFloat4(&this->data.color,
							   DirectX::XMVectorSet(newColor[0], newColor[1], newColor[2], newColor[3]));
	}

	if (data.contains("intensity")) {
		this->data.intensity = data["intensity"].get<float>();
	}
}

void PointLightObject::SaveToJson(nlohmann::json& data) {
	this->GameObject3D::SaveToJson(data);

	data["type"] = "PointLightObject";

	DirectX::XMVECTOR currentColor = DirectX::XMLoadFloat4(&this->data.color);
	data["color"] = {currentColor.m128_f32[0], currentColor.m128_f32[1], currentColor.m128_f32[2],
					 currentColor.m128_f32[3]};

	data["intensity"] = this->data.intensity;
}

void PointLightObject::SetShadowResolution(size_t res) {
	this->shadowViewPort = {
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<FLOAT>(res),
		.Height = static_cast<FLOAT>(res),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};

	this->resolutionChanged = true;
}

void PointLightObject::SetDepthBuffers(ID3D11Device* device) {
	this->shadowCubeMap.Init(device, this->shadowViewPort.Width);
	this->resolutionChanged = false;
}

void PointLightObject::ShowInHierarchy() {
	this->GameObject3D::ShowInHierarchy();

	ImGui::Text("PointLight");
	float intensity = this->data.intensity;
	ImGui::SliderFloat("Intensity", &intensity, 0.0f, 200.0f);
	this->data.intensity = intensity;
}

void SetCameraOrientation(CameraObject* cam, DirectX::XMFLOAT3 forwardF, DirectX::XMFLOAT3 upF) {
    using namespace DirectX;
    XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&forwardF));
    XMVECTOR up = XMVector3Normalize(XMLoadFloat3(&upF));
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
    up = XMVector3Cross(forward, right); // orthonormalize

    XMMATRIX worldMat = {
        right, // row/col depends on your conventions — this builds basis columns
        up,
        forward,
        XMVectorSet(0,0,0,1)
    };

    XMVECTOR q = XMQuaternionRotationMatrix(worldMat);
    XMFLOAT4 quat;
    XMStoreFloat4(&quat, q);

    cam->transform.SetRotationQuaternion(quat); // or convert quaternion to whatever your transform API needs
}
