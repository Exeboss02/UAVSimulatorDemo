#include "gameObjects/transform.h"

Transform::Transform() : position({}), quaternion(DirectX::XMQuaternionIdentity()), scale({1, 1, 1}) {}

Transform::Transform(DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion, DirectX::XMVECTOR scale)
	: position(position), quaternion(DirectX::XMQuaternionNormalize(quaternion)), scale(scale) {}

Transform::Transform(DirectX::XMVECTOR position, float roll, float pitch, float yaw, DirectX::XMVECTOR scale)
	: position(position),
	  quaternion(DirectX::XMQuaternionRotationRollPitchYawFromVector(
		  {DirectX::XMConvertToRadians(roll), DirectX::XMConvertToRadians(pitch), DirectX::XMConvertToRadians(yaw)})),
	  scale(scale) {}

void Transform::SetPosition(DirectX::XMVECTOR position) { this->position = position; }

void Transform::SetRotationRPY(float roll, float pitch, float yaw) {
	this->quaternion = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

void Transform::SetRotationRPY(DirectX::XMVECTOR rollPitchYaw) {
	this->quaternion = DirectX::XMQuaternionRotationRollPitchYawFromVector(rollPitchYaw);
}

void Transform::SetRotationQuaternion(DirectX::XMVECTOR quaternion) {
	this->quaternion = DirectX::XMQuaternionNormalize(quaternion);
}

void Transform::SetScale(DirectX::XMVECTOR scale) { this->scale = scale; }

void Transform::Move(DirectX::XMVECTOR move) { this->position = DirectX::XMVectorAdd(this->position, move); }

void Transform::Move(DirectX::XMVECTOR direction, float speed) { this->Move(DirectX::XMVectorScale(direction, speed)); }

void Transform::Rotate(float x, float y, float z) {
	this->RotateQuaternion(DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
}

void Transform::RotateQuaternion(DirectX::XMVECTOR quaternion) {
	this->quaternion = DirectX::XMQuaternionMultiply(this->quaternion, quaternion);
}

DirectX::XMVECTOR Transform::GetPosition() const { return this->position; }

DirectX::XMVECTOR Transform::GetRotationQuaternion() const { return this->quaternion; }

DirectX::XMVECTOR Transform::GetDirectionVector() const {
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), this->quaternion);
}

DirectX::XMVECTOR Transform::GetRightVector() const 
{
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), this->quaternion);
}

DirectX::XMVECTOR Transform::GetUpVector() const 
{ 
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
