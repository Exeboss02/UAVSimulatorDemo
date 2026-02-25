#include "gameObjects/transform.h"

#include "gameObjects/gameObject3D.h"
#include <stdexcept>

Transform::Transform(GameObject3D* gameObject)
	: position({}), quaternion(DirectX::XMQuaternionIdentity()), scale({1, 1, 1}), myGameObject(gameObject) {}

Transform::Transform(GameObject3D* gameObject, DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion,
					 DirectX::XMVECTOR scale)
	: position(position), quaternion(DirectX::XMQuaternionNormalize(quaternion)), scale(scale),
	  myGameObject(gameObject) {}

Transform::Transform(GameObject3D* gameObject, DirectX::XMVECTOR position, float roll, float pitch, float yaw,
					 DirectX::XMVECTOR scale)
	: position(position),
	  quaternion(DirectX::XMQuaternionRotationRollPitchYawFromVector(
		  {DirectX::XMConvertToRadians(roll), DirectX::XMConvertToRadians(pitch), DirectX::XMConvertToRadians(yaw)})),
	  scale(scale), myGameObject(gameObject) {}

void Transform::SetPosition(DirectX::XMVECTOR position) {
	this->position = position;
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::SetPosition(float x, float y, float z) { this->SetPosition(DirectX::XMVectorSet(x, y, z, 0)); }

void Transform::SetRotationRPY(float roll, float pitch, float yaw) {
	this->quaternion = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::SetRotationRPY(DirectX::XMVECTOR rollPitchYaw) {
	this->quaternion = DirectX::XMQuaternionRotationRollPitchYawFromVector(rollPitchYaw);
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::SetDirection(float x, float y, float z) {

	DirectX::XMVECTOR dir = DirectX::XMVectorSet(x, y, z, 0);

	this->SetDirection(dir);
}

void Transform::SetDirection(DirectX::XMVECTOR direction) {
	direction = DirectX::XMVector3Normalize(direction);

	DirectX::XMVECTOR right =
		DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0, 1, 0, 0), direction));

	DirectX::XMVECTOR up = DirectX::XMVector3Cross(direction, right);

	DirectX::XMMATRIX world = DirectX::XMMATRIX(right, up, direction, DirectX::XMVectorSet(0, 0, 0, 1));

	quaternion = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionRotationMatrix(world));

	this->myGameObject->SetHasMovedRecursive();
}

void Transform::SetRotationQuaternion(DirectX::XMVECTOR quaternion) {
	this->quaternion = DirectX::XMQuaternionNormalize(quaternion);
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::SetScale(DirectX::XMVECTOR scale) {
	this->scale = scale;
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::SetScale(float x, float y, float z) { this->SetScale(DirectX::XMVectorSet(x, y, z, 1)); }

void Transform::Move(DirectX::XMVECTOR move) {
	this->position = DirectX::XMVectorAdd(this->position, move);
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::Move(DirectX::XMVECTOR direction, float speed) {
	this->Move(DirectX::XMVectorScale(direction, speed));
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::Rotate(float x, float y, float z) {
	this->RotateQuaternion(DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
	this->myGameObject->SetHasMovedRecursive();
}

void Transform::RotateQuaternion(DirectX::XMVECTOR quaternion) {
	this->quaternion = DirectX::XMQuaternionMultiply(this->quaternion, quaternion);
	this->myGameObject->SetHasMovedRecursive();
}

DirectX::XMVECTOR Transform::GetPosition() const { return this->position; }

DirectX::XMVECTOR Transform::GetRotationQuaternion() const { return this->quaternion; }

DirectX::XMVECTOR Transform::GetDirectionVector() const {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), this->quaternion);
}

DirectX::XMVECTOR Transform::GetRightVector() const {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), this->quaternion);
}

DirectX::XMVECTOR Transform::GetUpVector() const {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), this->quaternion);
}

DirectX::XMVECTOR Transform::GetScale() const { return this->scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix(bool inverseTranspose) const {

	// Create the scaling, rotation, and translation matrices
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScalingFromVector(this->GetScale());
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(this->GetRotationQuaternion());
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(this->GetPosition());
	// Combine the matrices to create the world matrix (scale * rotation * translation)
	DirectX::XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	// Transpose the matrix if needed (depends on the target platform/GPU conventions)
	// worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);

	if (inverseTranspose) {
		worldMatrix = DirectX::XMMatrixInverse(nullptr, worldMatrix);
		worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
	}

	// Store the result in a XMFLOAT4X4
	DirectX::XMFLOAT4X4 worldMatrixFloat4x4;
	DirectX::XMStoreFloat4x4(&worldMatrixFloat4x4, worldMatrix);

	return worldMatrixFloat4x4;
}

DirectX::XMFLOAT4X4 Transform::GetViewMatrix() const {
	DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0),
														 this->GetRotationQuaternion()); // Local forward
	DirectX::XMVECTOR up =
		DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), this->GetRotationQuaternion()); // Local up

	DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(this->GetPosition(), forward, up);

	DirectX::XMFLOAT4X4 matrixfloat;
	DirectX::XMStoreFloat4x4(&matrixfloat, matrix);

	return matrixfloat;
}

DirectX::XMVECTOR Transform::GetCameraRotationQuaternion(float yawDegrees, float pitchDegrees) {
	// Convert to radians.
	float yawRad = DirectX::XMConvertToRadians(yawDegrees);
	float pitchRad = DirectX::XMConvertToRadians(pitchDegrees);

	DirectX::XMVECTOR vUp = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMVECTOR vDefaultR = DirectX::XMVectorSet(0, 0, 1, 0);

	// Build yaw quaternion around world-up.
	DirectX::XMVECTOR qYaw = DirectX::XMQuaternionRotationAxis(vUp, yawRad);

	// Rotate the default right axis by that yaw to get the local right.
	DirectX::XMVECTOR vRight = DirectX::XMVector3Rotate(vDefaultR, qYaw);

	// Build pitch quaternion around the local right axis.
	DirectX::XMVECTOR qPitch = DirectX::XMQuaternionRotationAxis(vRight, -pitchRad);

	// Combine: first yaw, then pitch.
	DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(qPitch, qYaw);

	// Normalize for safety.
	return DirectX::XMQuaternionNormalize(qResult);
}

DirectX::XMVECTOR Transform::GetGlobalPosition() {
	if (this->recalculateGlobalPosition) {
		this->globalPosition = GetDecomposedWorldMatrix(TransformComponent::TRANSLATION);
		this->recalculateGlobalPosition = false;
	}

	return this->globalPosition;
}

DirectX::XMVECTOR Transform::GetGlobalRotation() {
	if (this->recalculateGlobalRotation) {
		this->globalRotation = DirectX::XMQuaternionNormalize(GetDecomposedWorldMatrix(TransformComponent::ROTATION));
		this->recalculateGlobalRotation = false;
	}

	return this->globalRotation;
}

DirectX::XMVECTOR Transform::GetGlobalScale() {
	if (this->recalculateGlobalScale) {
		this->globalScale = GetDecomposedWorldMatrix(TransformComponent::SCALE);
		this->recalculateGlobalScale = false;
	}

	return this->globalScale;
}

DirectX::XMVECTOR Transform::GetGlobalForward() {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), GetGlobalRotation());
}

DirectX::XMVECTOR Transform::GetGlobalRight() {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), GetGlobalRotation());
}

DirectX::XMVECTOR Transform::GetGlobalUp() {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), GetGlobalRotation());
}

void Transform::HasMoved() {
	this->recalculateGlobalWorldMatrix = true;
	this->recalculateGlobalWorldMatrixInverseTransposed = true;
	this->recalculateGlobalPosition = true;
	this->recalculateGlobalRotation = true;
	this->recalculateGlobalScale = true;
}

DirectX::XMMATRIX Transform::GetGlobalWorldMatrix(bool inverseTranspose) {
	if (inverseTranspose) {
		if (this->recalculateGlobalWorldMatrixInverseTransposed) {
			this->globalWorldMatrixInverseTransposed = this->myGameObject->GetGlobalWorldMatrixRecursive(true);
			this->recalculateGlobalWorldMatrixInverseTransposed = false;
		}

		return this->globalWorldMatrixInverseTransposed;
	} else {
		if (this->recalculateGlobalWorldMatrix) {
			this->globalWorldMatrix = this->myGameObject->GetGlobalWorldMatrixRecursive(false);
			this->recalculateGlobalWorldMatrix = false;
		}

		return this->globalWorldMatrix;
	}
}

DirectX::XMVECTOR Transform::GetDecomposedWorldMatrix(const TransformComponent& component) {
	DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotationQuaternion;
	DirectX::XMVECTOR translation;

	DirectX::XMMatrixDecompose(&scale, &rotationQuaternion, &translation, GetGlobalWorldMatrix(false));

	switch (component) {
	case TransformComponent::SCALE:
		return scale;
	case TransformComponent::ROTATION:
		return rotationQuaternion;
	case TransformComponent::TRANSLATION:
		return translation;
	default:
		throw std::runtime_error("Failed Transform::GetDecomposedWorldMatrix()");
	}
}
