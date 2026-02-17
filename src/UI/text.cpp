#include "UI/text.h"
#include "UI/textRenderer.h"
#include "utilities/logger.h"
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

	// Text content — use a persistent edit buffer so ImGui can edit across frames
	if (this->editBuffer.empty()) {
		this->editBuffer.resize(512);
		std::fill(this->editBuffer.begin(), this->editBuffer.end(), 0);
		std::strncpy(this->editBuffer.data(), this->text.c_str(), this->editBuffer.size() - 1);
	}

	if (ImGui::InputText("Text", this->editBuffer.data(), static_cast<int>(this->editBuffer.size()))) {
		this->text = std::string(this->editBuffer.data());
		this->SetName(this->text);
	}

	// Font — persistent buffer so we can show and edit the font name (default when empty)
	if (this->fontEditBuffer.empty()) {
		this->fontEditBuffer.resize(128);
		std::fill(this->fontEditBuffer.begin(), this->fontEditBuffer.end(), 0);
		const char* initial = this->font.empty() ? "default" : this->font.c_str();
		std::strncpy(this->fontEditBuffer.data(), initial, this->fontEditBuffer.size() - 1);
		// if font member was empty, set it to default so renderer uses it
		if (this->font.empty()) this->font = "default";
	}

	if (ImGui::InputText("Font", this->fontEditBuffer.data(), static_cast<int>(this->fontEditBuffer.size()))) {
		this->font = std::string(this->fontEditBuffer.data());
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

void Text::Update(float dt) {
	// Update transform mapping
	Widget::Update(dt);
}

void Text::Draw() {
	if (!this->IsVisible()) {
		Logger::Log("Text::Draw: widget not visible:", this->GetName());
		return;
	}

	UI::TextRenderer::GetInstance().SubmitText(
		this->text, this->GetPosition(), this->GetSize().y,
		DirectX::XMFLOAT4(this->color.x, this->color.y, this->color.z, this->color.w), this->font);
	Logger::Log("Text::Draw: submitted text:", this->text, "font:", this->font, "widget:", this->GetName());

	// Draw children (if any)
	Widget::Draw();
}
