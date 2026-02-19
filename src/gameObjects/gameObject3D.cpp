#include "gameObjects/gameObject3D.h"

GameObject3D::GameObject3D() : transform(this)
{
}

void GameObject3D::Tick()
{
	
}

DirectX::XMMATRIX GameObject3D::GetGlobalWorldMatrixRecursive(bool inverseTranspose) const
{
	DirectX::XMFLOAT4X4 localWorldXMFLOAT4X4 = this->transform.GetWorldMatrix(inverseTranspose);
	DirectX::XMMATRIX localWorldMatrix = DirectX::XMLoadFloat4x4(&localWorldXMFLOAT4X4);

	if (this->GetParent().expired()) {
		return localWorldMatrix;
	}
	else {
		return localWorldMatrix * this->GetParent().lock()->GetGlobalWorldMatrixRecursive(inverseTranspose);
	}
}

void GameObject3D::SetHasMovedRecursive() {
	this->transform.HasMoved();

	for (auto& child : this->GetChildren()) {
		child.lock()->SetHasMovedRecursive();
	}
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
