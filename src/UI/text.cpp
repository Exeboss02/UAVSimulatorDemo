#include "UI/text.h"
#include "UI/textRenderer.h"
#include "utilities/logger.h"
#include <nlohmann/json.hpp>

using namespace UI;

void Text::SetText(const std::string& t) { 
	this->text = t; 
	this->CalculateTruePosition();
}
std::string Text::GetText() const { return this->text; }

void Text::SetFont(const std::string& f) { 
	this->font = f; 
	this->CalculateTruePosition();
}
std::string Text::GetFont() const { return this->font; }

void Text::SetColor(const DirectX::XMFLOAT4& c) { this->color = c; }
DirectX::XMFLOAT4 Text::GetColor() const { return this->color; }

Vec2 UI::Text::GetSize() const { 
	Vec2 size = Vec2(UI::TextRenderer::GetInstance().MeasureString(this->text, this->GetFontSize(), this->font), this->GetFontSize());
	return size;
}

void Text::ShowInHierarchy() {
	// Show base GameObject inspector (name, active, delete/reparent)
	this->GameObject::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Widget settings:");

	// Position
	Vec2 pos = this->GetPosition();
	if (ImGui::InputFloat2("Position", &pos.x)) {
		this->SetPosition(pos);
	}

	// Replace Size control with Font Size for text widgets
	if (ImGui::DragFloat("Font Size", &this->fontSize, 1.0f, 1.0f, 512.0f)) {
		if (this->fontSize < 1.0f) this->fontSize = 1.0f;
	}

	// Visibility / enabled
	bool vis = this->IsVisible();
	if (ImGui::Checkbox("Visible", &vis)) {
		this->SetVisible(vis);
	}

	bool en = this->isEnabled();
	if (ImGui::Checkbox("Enabled", &en)) {
		this->SetEnabled(en);
	}

	// Z-index
	int z = this->GetZIndex();
	if (ImGui::InputInt("Z Index", &z)) {
		this->SetZIndex(z);
	}

	ImGui::Separator();

	ImGui::Text("Text settings:");

	// Text content — use a persistent edit buffer so ImGui can edit across frames
	if (this->editBuffer.empty()) {
		this->editBuffer.resize(512);
		std::fill(this->editBuffer.begin(), this->editBuffer.end(), 0);
		std::strncpy(this->editBuffer.data(), this->text.c_str(), this->editBuffer.size() - 1);
	}

	ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_None;
	textFlags |= ImGuiInputTextFlags_EnterReturnsTrue;

	if (ImGui::InputText("Text", this->editBuffer.data(), static_cast<int>(this->editBuffer.size()), textFlags)) {
		this->text = std::string(this->editBuffer.data());
	} else if (ImGui::IsItemDeactivatedAfterEdit()) {
		this->text = std::string(this->editBuffer.data());
	}

	// Font — persistent buffer so we can show and edit the font name (default when empty)
	if (this->fontEditBuffer.empty()) {
		this->fontEditBuffer.resize(128);
		std::fill(this->fontEditBuffer.begin(), this->fontEditBuffer.end(), 0);
		const char* initial = this->font.empty() ? "Lucida Console" : this->font.c_str();
		std::strncpy(this->fontEditBuffer.data(), initial, this->fontEditBuffer.size() - 1);
		// if font member was empty, set it to default so renderer uses it
		if (this->font.empty()) this->font = "Lucida Console";
	}

	ImGuiInputTextFlags fontFlags = ImGuiInputTextFlags_None;
	fontFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("Font", this->fontEditBuffer.data(), static_cast<int>(this->fontEditBuffer.size()),
						 fontFlags)) {
		this->font = std::string(this->fontEditBuffer.data());
	} else if (ImGui::IsItemDeactivatedAfterEdit()) {
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
}

void Text::SetFontSize(float s) { 
	this->fontSize = s; 
	this->CalculateTruePosition();
}
float Text::GetFontSize() const { return this->fontSize; }

void Text::SetRightAligned(bool v) { this->rightAligned = v; }
bool Text::IsRightAligned() const { return this->rightAligned; }

void Text::SetMaxWidth(float w) { this->maxWidth = w; }
float Text::GetMaxWidth() const { return this->maxWidth; }

void Text::LoadFromJson(const nlohmann::json& data) {
	this->Widget::LoadFromJson(data);

	if (data.contains("text")) this->SetText(data.at("text").get<std::string>());
	if (data.contains("font")) this->SetFont(data.at("font").get<std::string>());
	if (data.contains("color") && data["color"].is_array() && data["color"].size() == 4) {
		DirectX::XMFLOAT4 col{data["color"][0].get<float>(), data["color"][1].get<float>(),
							  data["color"][2].get<float>(), data["color"][3].get<float>()};
		this->SetColor(col);
	}

	if (data.contains("fontSize")) this->SetFontSize(data.at("fontSize").get<float>());
}

void Text::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["type"] = "UI::Text";
	data["text"] = this->GetText();
	data["font"] = this->GetFont();
	auto c = this->GetColor();
	data["color"] = {c.x, c.y, c.z, c.w};
	data["fontSize"] = this->GetFontSize();
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

	UI::Vec2 pos = this->GetPosition();
	if (this->rightAligned && this->maxWidth > 0.0f) {
		float measured = UI::TextRenderer::GetInstance().MeasureString(this->text, this->GetFontSize(), this->font);

		//float x = pos.x + std::max(0.0f, this->maxWidth - measured);
		//UI::Vec2 p{x, y};
		UI::TextRenderer::GetInstance().SubmitText(
			this->text, pos, this->GetFontSize(),
			DirectX::XMFLOAT4(this->color.x, this->color.y, this->color.z, this->color.w), this->font,
			this->GetZIndex());
	} else {
		UI::TextRenderer::GetInstance().SubmitText(
			this->text, pos, this->GetFontSize(),
			DirectX::XMFLOAT4(this->color.x, this->color.y, this->color.z, this->color.w), this->font,
			this->GetZIndex());
	}

	// Draw children (if any)
	Widget::Draw();
}
