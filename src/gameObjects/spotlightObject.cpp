#include "gameObjects/spotlightObject.h"
#include "gameObjects/cameraObject.h"

SpotlightObject::SpotlightObject() {
	this->data = {};
	DirectX::XMStoreFloat3(&this->data.position, DirectX::XMVectorSet(0, 0, 0, 0));
	DirectX::XMStoreFloat3(&this->data.direction, DirectX::XMVectorSet(0, 0, 0, 0));
	DirectX::XMStoreFloat4(&this->data.color, DirectX::XMVectorSet(1, 1, 1, 1));

	this->data.spotCosAngleRadians = DirectX::XMConvertToRadians(120);

	Logger::Log("Created a spotlight.");
}

SpotlightObject::SpotLightContainer SpotlightObject::GetSpotLightData() const { 
	return this->data;
}

float SpotlightObject::GetAngle() const { 
	if (this->camera.expired()) {
		std::string error = "Lights shadow camera has been killed unexpectedly";
		Logger::Error(error);
		throw std::runtime_error(error);
	}
	return this->camera.lock()->GetFov();
}

void SpotlightObject::Start() {
	RenderQueue::AddLightObject(this->GetPtr());

	// Attach camera for shadowpass
	this->camera = this->factory->CreateGameObjectOfType<CameraObject>();
	this->camera.lock()->SetParent(this->GetPtr());
	this->camera.lock()->SetFov(90);
	this->camera.lock()->SetAspectRatio(1 / 1);
	this->camera.lock()->SetFarPlane(20.);
}

void SpotlightObject::Tick() {
	// feels bad doing this every frame
	DirectX::XMStoreFloat3(&this->data.position, this->transform.GetGlobalPosition());
	DirectX::XMStoreFloat3(&this->data.direction, this->transform.GetGlobalForward());

	if (this->camera.expired()) {
		std::string error = "Spotlight shadowcamera has been killed";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	auto cameraLocked = this->camera.lock();

	this->data.spotCosAngleRadians = cos(DirectX::XMConvertToRadians(cameraLocked->GetFov()) / 2);

	auto viewProj = cameraLocked->GetCameraMatrix(true).viewProjectionMatrix;

	this->data.viewProjectionMatrix = viewProj;
}

void SpotlightObject::LoadFromJson(const nlohmann::json& data) {
	this->GameObject3D::LoadFromJson(data);

	if (data.contains("color")) {
		auto newColor = data["color"];
		DirectX::XMStoreFloat4(&this->data.color,
							   DirectX::XMVectorSet(newColor[0], newColor[1], newColor[2], newColor[3]));
	}

	if (data.contains("intensity")) {
		this->data.intensity = data["intensity"].get<float>();
	}

	if (data.contains("angleDegrees")) {
		this->camera.lock()->SetFov(data["angleDegrees"].get<float>());
	}
}

void SpotlightObject::SaveToJson(nlohmann::json& data) {
	this->GameObject3D::SaveToJson(data);

	data["type"] = "SpotlightObject";

	DirectX::XMVECTOR currentColor = DirectX::XMLoadFloat4(&this->data.color);
	data["color"] = {currentColor.m128_f32[0], currentColor.m128_f32[1], currentColor.m128_f32[2],
					 currentColor.m128_f32[3]};

	data["intensity"] = this->data.intensity;

	data["angleDegrees"] = DirectX::XMConvertToDegrees(this->data.spotCosAngleRadians);
}

void SpotlightObject::SetAngle(float angle) {
	if (this->camera.expired()) {
		std::string error = "Lights shadow camera has been killed unexpectedly";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	if (angle > 120) {
		Logger::Warn("Spotlight angle is more than 120 degrees, setting angle to 120");
		angle = 120;
	}

	this->camera.lock()->SetFov(angle);
}

void SpotlightObject::SetIntensity(float intensity) { this->data.intensity = intensity; }

void SpotlightObject::ShowInHierarchy() 
{
	this->GameObject3D::ShowInHierarchy();

	ImGui::Text("Spotlight");
	float intensity = this->data.intensity;
	ImGui::SliderFloat("Intensity", &intensity, 0.0f, 200.0f);
	this->data.intensity = intensity;
}
