#include "UI/button.h"

#include "core/input/inputManager.h"
#include <nlohmann/json.hpp>
#include <string>

void UI::Button::Start() {
	MeshObjData mesh = AssetManager::GetInstance().GetMeshObjData("quad/quad.glb:Mesh_0");
	this->SetMesh(mesh);
}

void UI::Button::Update(float dt) {
	if (!this->visible || !this->enabled) return;

	auto mp = InputManager::GetInstance().GetMousePosition();
	int mx = mp[0];
	int my = mp[1];
	Vec2 point{static_cast<float>(mx), static_cast<float>(my)};
	const bool hit = this->HitTest(point);

	if (hit && !this->hovered) {
		this->hovered = true;
		if (this->onHover) this->onHover();
	} else if (!hit && this->hovered) {
		this->hovered = false;
		if (this->onUnhover) this->onUnhover();
	}

	if (hit && InputManager::GetInstance().WasLMPressed()) {
		this->pressed = true;
		if (this->onPressed) this->onPressed();
	}

	if (hit && InputManager::GetInstance().WasLMReleased()) {
		this->pressed = false;
		if (this->onReleased) this->onReleased();
		if (hit && this->onClick) this->onClick();
	}

	Widget::Update(dt);
}

void UI::Button::SetOnClick(std::function<void()> callback) { this->onClick = std::move(callback); }

void UI::Button::SetOnPressed(std::function<void()> callback) { this->onPressed = std::move(callback); }

void UI::Button::SetOnReleased(std::function<void()> callback) { this->onReleased = std::move(callback); }

void UI::Button::SetOnHover(std::function<void()> callback) { this->onHover = std::move(callback); }

void UI::Button::SetOnUnhover(std::function<void()> callback) { this->onUnhover = std::move(callback); }

bool UI::Button::IsHovered() const { return this->hovered; }

bool UI::Button::isPressed() const { return this->pressed; }

void UI::Button::ShowInHierarchy() {
	// Show generic widget options first
	Widget::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Button settings:");

	// Label
	char buf[128];
	std::string cur = this->label;
	std::strncpy(buf, cur.c_str(), sizeof(buf));
	if (ImGui::InputText("Label", buf, sizeof(buf))) {
		this->label = std::string(buf);
		this->SetName(this->label);
	}

	// Interaction readouts
	ImGui::Text("Hovered: %s", this->hovered ? "true" : "false");
	ImGui::Text("Pressed: %s", this->pressed ? "true" : "false");

	ImGui::Separator();
}

void UI::Button::SetLabel(const std::string& l) { this->label = l; }

std::string UI::Button::GetLabel() const { return this->label; }

void UI::Button::LoadFromJson(const nlohmann::json& data) {
	// Load base widget fields
	this->Widget::LoadFromJson(data);

	if (data.contains("label")) this->SetLabel(data.at("label").get<std::string>());
}

void UI::Button::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["type"] = "UI::Button";
	data["label"] = this->GetLabel();
}
