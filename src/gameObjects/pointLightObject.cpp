#include "gameObjects/pointLightObject.h"

PointLightObject::PointLightObject() {
	this->data = {};
	DirectX::XMStoreFloat3(&this->data.position, DirectX::XMVectorSet(0, 0, 0, 0));
	DirectX::XMStoreFloat4(&this->data.color, DirectX::XMVectorSet(1, 1, 1, 1));

	Logger::Log("Created a pointLight.");
}

PointLightObject::PointLightContainer PointLightObject::GetPointLightData() {
	DirectX::XMStoreFloat3(&this->data.position, this->transform.GetGlobalPosition());

	for (size_t i = 0; i < 6; i++) {
		if (this->cameras[i].expired()) {
			Logger::Error("One of PointLight shadowcameras were expired");
			continue;
		}
		this->data.viewProjectionMatrix[i] = this->cameras[i].lock()->GetCameraMatrix(true).viewProjectionMatrix;
	}

	return this->data;
}

void SetCameraOrientation(CameraObject* cam, DirectX::XMFLOAT3 forwardF, DirectX::XMFLOAT3 upF);

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
	DirectX::XMStoreFloat3(&this->data.position, this->transform.GetGlobalPosition());

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

void PointLightObject::ShowInHierarchy() {
	this->GameObject3D::ShowInHierarchy();

	if (!DISABLE_IMGUI) {
		ImGui::Text("PointLight");
		float intensity = this->data.intensity;
		ImGui::SliderFloat("Intensity", &intensity, 0.0f, 200.0f);
		this->data.intensity = intensity;
	}

}

void PointLightObject::OnDestroy() {
	this->GameObject3D::OnDestroy();
	Logger::Error("You can't destroy lights atm :)");
	throw std::runtime_error("Fatal error in PointLight");
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

    cam->transform.SetRotationQuaternion(q); // or convert quaternion to whatever your transform API needs
}
