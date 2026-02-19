#include "UI/image.h"
#include "utilities/logger.h"
#include <nlohmann/json.hpp>
#include <DirectXMath.h>

void UI::Image::SetImage(const std::string& img) { this->image = img; }

std::string UI::Image::GetImage() const { return this->image; }

void UI::Image::ShowInHierarchy() {
	Widget::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Image settings:");

	if (this->imageEditBuffer.empty()) {
		this->imageEditBuffer.resize(512);
		std::fill(this->imageEditBuffer.begin(), this->imageEditBuffer.end(), 0);
		std::strncpy(this->imageEditBuffer.data(), this->image.c_str(), this->imageEditBuffer.size() - 1);
	}

	if (ImGui::InputText("Image", this->imageEditBuffer.data(), static_cast<int>(this->imageEditBuffer.size()))) {
		this->image = std::string(this->imageEditBuffer.data());
	}

	// Tint
	float col[4] = {this->tint.x, this->tint.y, this->tint.z, this->tint.w};
	if (ImGui::ColorEdit4("Tint", col)) {
		this->tint.x = col[0];
		this->tint.y = col[1];
		this->tint.z = col[2];
		this->tint.w = col[3];
	}

	ImGui::Separator();
}

void UI::Image::LoadFromJson(const nlohmann::json& data) {
	this->Widget::LoadFromJson(data);

	if (data.contains("image")) this->SetImage(data.at("image").get<std::string>());
	if (data.contains("tint") && data["tint"].is_array() && data["tint"].size() == 4) {
		this->tint.x = data["tint"][0].get<float>();
		this->tint.y = data["tint"][1].get<float>();
		this->tint.z = data["tint"][2].get<float>();
		this->tint.w = data["tint"][3].get<float>();
	}
}

void UI::Image::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["type"] = "UI::Image";
	data["image"] = this->GetImage();
	data["tint"] = {this->tint.x, this->tint.y, this->tint.z, this->tint.w};
}

void UI::Image::Update(float dt) { Widget::Update(dt); }

void UI::Image::Draw() {
	if (!this->IsVisible()) {
		Logger::Log("Image::Draw: widget not visible:", this->GetName());
		return;
	}

	Widget::Draw();
}

void UI::Image::SetTint(const DirectX::XMFLOAT4& c) { this->tint = c; }

DirectX::XMFLOAT4 UI::Image::GetTint() const { return this->tint; }
