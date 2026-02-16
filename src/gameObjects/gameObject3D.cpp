#include "gameObjects/gameObject3D.h"

GameObject3D::GameObject3D() : transform()
{
}

void GameObject3D::Tick()
{
	
}

DirectX::XMVECTOR GameObject3D::GetGlobalPosition() const
{
	return GetDecomposedWorldMatrix(TransformComponent::TRANSLATAION);
}

DirectX::XMVECTOR GameObject3D::GetGlobalRotation() const
{
	return GetDecomposedWorldMatrix(TransformComponent::ROTATION);
}

DirectX::XMVECTOR GameObject3D::GetGlobalScale() const
{
	return GetDecomposedWorldMatrix(TransformComponent::SCALE);
}

DirectX::XMMATRIX GameObject3D::GetGlobalWorldMatrix(bool inverseTranspose) const
{
	DirectX::XMFLOAT4X4 localWorldXMFLOAT4X4 = this->transform.GetWorldMatrix(inverseTranspose);
	DirectX::XMMATRIX localWorldMatrix = DirectX::XMLoadFloat4x4(&localWorldXMFLOAT4X4);

	if (this->GetParent().expired()) {
		return localWorldMatrix;
	}
	else {
		return localWorldMatrix * this->GetParent().lock()->GetGlobalWorldMatrix(inverseTranspose);
	}
}

DirectX::XMVECTOR GameObject3D::GetGlobalForward() const
{
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), GetGlobalRotation());
}

DirectX::XMVECTOR GameObject3D::GetGlobalRight() const 
{ 
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), GetGlobalRotation());
}

DirectX::XMVECTOR GameObject3D::GetGlobalUp() const
{
	return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), GetGlobalRotation());
}

void GameObject3D::LoadFromJson(const nlohmann::json& data) {
	this->GameObject::LoadFromJson(data);

	if (data.contains("transform")) {
		if (data["transform"].contains("position")) {
			auto position = data.at("transform").at("position");

			this->transform.SetPosition(DirectX::XMVectorSet(position[0], position[1], position[2], 1));
		}
		if (data["transform"].contains("rotation")) {
			auto rotation = data.at("transform").at("rotation");

			this->transform.SetRotationQuaternion(DirectX::XMVectorSet(rotation[0], rotation[1], rotation[2], rotation[3]));
		}
		if (data["transform"].contains("scale")) {
			auto scale = data.at("transform").at("scale");

			this->transform.SetScale(DirectX::XMVectorSet(scale[0], scale[1], scale[2], 1));
		}
	}
}

void GameObject3D::SaveToJson(nlohmann::json& data)
{
	this->GameObject::SaveToJson(data);

	data["type"] = "GameObject3D";
	data["transform"]["position"] = { this->transform.GetPosition().m128_f32[0], this->transform.GetPosition().m128_f32[1], this->transform.GetPosition().m128_f32[2] };
	data["transform"]["rotation"] = { this->transform.GetRotationQuaternion().m128_f32[0], this->transform.GetRotationQuaternion().m128_f32[1], this->transform.GetRotationQuaternion().m128_f32[2], this->transform.GetRotationQuaternion().m128_f32[3] };
	data["transform"]["scale"] = { this->transform.GetScale().m128_f32[0], this->transform.GetScale().m128_f32[1], this->transform.GetScale().m128_f32[2] };
}

void GameObject3D::ShowInHierarchy() 
{ 
	this->GameObject::ShowInHierarchy(); 

	ImGui::Text("GameObject3D"); 

	// Position
	DirectX::XMVECTOR pos = this->transform.GetPosition();
	float newPos[3] = {pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]};
	ImGui::InputFloat3("Local Position", newPos);
	this->transform.SetPosition(DirectX::XMVectorSet(newPos[0], newPos[1], newPos[2], 1.0f));

	// Scale
	DirectX::XMVECTOR scale = this->transform.GetScale();
	float newScale[3] = {scale.m128_f32[0], scale.m128_f32[1], scale.m128_f32[2]};
	ImGui::InputFloat3("Local Scale", newScale);
	this->transform.SetScale(DirectX::XMVectorSet(newScale[0], newScale[1], newScale[2], 1.0f));
}

DirectX::XMVECTOR GameObject3D::GetDecomposedWorldMatrix(const TransformComponent& component) const
{
	DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotationQuaternion;
	DirectX::XMVECTOR translation;

	DirectX::XMMatrixDecompose(&scale, &rotationQuaternion, &translation, GetGlobalWorldMatrix(false));
	
	switch (component) {
	case TransformComponent::SCALE:
		return scale;
	case TransformComponent::ROTATION:
		return rotationQuaternion;
	case TransformComponent::TRANSLATAION:
		return translation;
	default:
		throw std::runtime_error("Failed GameObject3D::GetDecomposedWorldMatrix()");
	}
}
