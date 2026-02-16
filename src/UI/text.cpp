#include "UI/text.h"
#include "imgui.h"
#include <nlohmann/json.hpp>

using namespace UI;

void Text::SetText(const std::string& t) { this->text = t; }
std::string Text::GetText() const { return this->text; }

void Text::SetFont(const std::string& f) { this->font = f; }
std::string Text::GetFont() const { return this->font; }

void Text::SetColor(const DirectX::XMFLOAT4& c) { this->color = c; }
DirectX::XMFLOAT4 Text::GetColor() const { return this->color; }

void Text::ShowInHierarchy() {
	Widget::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Text settings:");

	// Text content
	char buf[512];
	std::strncpy(buf, this->text.c_str(), sizeof(buf));
	if (ImGui::InputText("Text", buf, sizeof(buf))) {
		this->text = std::string(buf);
		this->SetName(this->text);
	}

	// Font
	char fbuf[256];
	std::strncpy(fbuf, this->font.c_str(), sizeof(fbuf));
	if (ImGui::InputText("Font", fbuf, sizeof(fbuf))) {
		this->font = std::string(fbuf);
	}

	// Color
	float col[4] = {this->color.x, this->color.y, this->color.z, this->color.w};
	if (ImGui::ColorEdit4("Color", col)) {
		this->color.x = col[0];
		this->color.y = col[1];
		this->color.z = col[2];
		this->color.w = col[3];
	}

	ImGui::Separator();
}

void Text::LoadFromJson(const nlohmann::json& data) {
	this->Widget::LoadFromJson(data);

	if (data.contains("text")) this->SetText(data.at("text").get<std::string>());
	if (data.contains("font")) this->SetFont(data.at("font").get<std::string>());
	if (data.contains("color") && data["color"].is_array() && data["color"].size() == 4) {
		DirectX::XMFLOAT4 col{data["color"][0].get<float>(), data["color"][1].get<float>(),
							  data["color"][2].get<float>(), data["color"][3].get<float>()};
		this->SetColor(col);
	}
}

void Text::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["type"] = "UI::Text";
	data["text"] = this->GetText();
	data["font"] = this->GetFont();
	auto c = this->GetColor();
	data["color"] = {c.x, c.y, c.z, c.w};
}
