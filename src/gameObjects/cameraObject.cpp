#include "gameObjects/cameraObject.h"
#include "DirectXMath.h"

CameraObject* CameraObject::mainCamera = nullptr;
size_t CameraObject::cameraIdCounter = 0;

CameraObject::CameraObject()
	: fieldOfView(80.0f), cameraId(CameraObject::cameraIdCounter++), farPlane(1000.), nearPlane(0.1),
	  aspectRatio(16. / 9.) {
	if (!this->mainCamera) {
		SetMainCamera();
	}

	Logger::Log("Successfully created a camera.");
}

void CameraObject::Tick() {
}

void CameraObject::LateTick() { /*UpdateCameraMatrix();*/ }

void CameraObject::Start() {}

void CameraObject::SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }

CameraObject::CameraMatrixContainer CameraObject::GetCameraMatrix(bool transposeViewProjMult) {
	CameraMatrixContainer matrix{};

	matrix.cameraPosition = this->transform.GetGlobalPosition();

	// View Projection Matrix
	DirectX::XMVECTOR globalRotation = this->transform.GetGlobalRotation();

	DirectX::XMVECTOR focusPos = DirectX::XMVectorAdd(
		matrix.cameraPosition, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), globalRotation));
	DirectX::XMVECTOR upDir = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), globalRotation);
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(matrix.cameraPosition, focusPos, upDir);

	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(this->fieldOfView), this->aspectRatio, this->nearPlane, this->farPlane);
	DirectX::XMMATRIX viewProjMatrix;
	if (transposeViewProjMult) {
		viewProjMatrix = DirectX::XMMatrixMultiplyTranspose(viewMatrix, projMatrix);
	} else {
		viewProjMatrix = viewMatrix * projMatrix;
	}

	DirectX::XMStoreFloat4x4(&matrix.viewProjectionMatrix, viewProjMatrix);

	return matrix;
}

DirectX::XMMATRIX CameraObject::GetViewProjectionMatrix(bool transposeViewProjMult) {
	// View Projection Matrix
	DirectX::XMVECTOR globalRotation = this->transform.GetGlobalRotation();

	DirectX::XMVECTOR position = this->transform.GetGlobalPosition();

	DirectX::XMVECTOR focusPos =
		DirectX::XMVectorAdd(position, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), globalRotation));

	DirectX::XMVECTOR upDir = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), globalRotation);
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(position, focusPos, upDir);

	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(this->fieldOfView), this->aspectRatio, this->nearPlane, this->farPlane);
	DirectX::XMMATRIX viewProjMatrix;
	if (transposeViewProjMult) {
		viewProjMatrix = DirectX::XMMatrixMultiplyTranspose(viewMatrix, projMatrix);
	} else {
		viewProjMatrix = viewMatrix * projMatrix;
	}

	return viewProjMatrix;
}

DirectX::XMMATRIX CameraObject::GetViewMatrix() {
	DirectX::XMVECTOR globalRotation = this->transform.GetGlobalRotation();
	DirectX::XMVECTOR position = this->transform.GetGlobalPosition();

	DirectX::XMVECTOR focusPos =
		DirectX::XMVectorAdd(position, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), globalRotation));
	DirectX::XMVECTOR upDir = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), globalRotation);
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(position, focusPos, upDir);
	return viewMatrix;
}

DirectX::XMMATRIX CameraObject::GetProjectionMatrix() const {
	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(this->fieldOfView), this->aspectRatio, this->nearPlane, this->farPlane);
	return projMatrix;
}

void CameraObject::SetMainCamera() { 
	this->mainCamera = this;
	RenderQueue::RecalculateDynamic();
}

void CameraObject::SetFov(float fov) { this->fieldOfView = fov; }

void CameraObject::SetFarPlane(float farPlane) { this->farPlane = farPlane; }

void CameraObject::SetNearPlane(float nearPlane) { this->nearPlane = nearPlane; }

CameraObject& CameraObject::GetMainCamera() {
	if (!CameraObject::mainCamera) {
		throw std::runtime_error("No active camera!");
	}

	return *CameraObject::mainCamera;
}

float CameraObject::GetFov() const { return this->fieldOfView; }

float CameraObject::GetFarPlane() const { return this->farPlane; }

float CameraObject::GetNearPlane() const { return this->nearPlane; }

float CameraObject::GetAspectRatio() const { return this->aspectRatio; }

void CameraObject::LoadFromJson(const nlohmann::json& data) {
	this->GameObject3D::LoadFromJson(data);

	if (data.contains("fov")) {
		this->fieldOfView = data.at("fov").get<float>();
	}
}

void CameraObject::SaveToJson(nlohmann::json& data) {
	this->GameObject3D::SaveToJson(data);

	data["type"] = "CameraObject";
	data["fov"] = this->fieldOfView;
}

void CameraObject::ShowInHierarchy() {
	this->GameObject3D::ShowInHierarchy();

	if (!DISABLE_IMGUI) {
		ImGui::Text("CameraObject");

		if (ImGui::Button("Set Main")) {
			SetMainCamera();
		}
		ImGui::SliderFloat("FOV", &this->fieldOfView, 1.0f, 120.0f);
	}
}

size_t CameraObject::GetCameraId() { return this->cameraId; }

// void CameraObject::UpdateCameraMatrix() {
//	// Position
//
//	this->cameraMatrix.cameraPosition = GetGlobalPosition();
//
//	// View Projection Matrix
//
//	DirectX::XMVECTOR globalRotation = GetGlobalRotation();
//
//	DirectX::XMVECTOR focusPos = DirectX::XMVectorAdd(
//		this->cameraMatrix.cameraPosition, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), globalRotation));
//	DirectX::XMVECTOR upDir = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), globalRotation);
//	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(this->cameraMatrix.cameraPosition, focusPos, upDir);
//
//	float tempAspectRatio = 16.0f / 9.0f;
//	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(this->fieldOfView),
//																	 tempAspectRatio, 0.1f, 1000.0f);
//
//	DirectX::XMMATRIX viewProjMatrix = viewMatrix * projMatrix;
//
//	DirectX::XMStoreFloat4x4(&this->cameraMatrix.viewProjectionMatrix, viewProjMatrix);
// }