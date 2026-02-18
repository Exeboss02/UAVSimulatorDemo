#include "UI/button.h"

#include "core/assetManager.h"
#include "core/input/inputManager.h"
#include "rendering/texture.h"
#include "rendering/unlitMaterial.h"
#include "utilities/logger.h"
#include <nlohmann/json.hpp>
#include <string>

void UI::Button::Start() {
	MeshObjData mesh = AssetManager::GetInstance().GetMeshObjData("quad/quad.glb:Mesh_0");

	// Diagnostics: ensure mesh data is valid
	if (mesh.GetMesh().expired()) {
		Logger::Error("Button::Start: quad mesh data expired or not loaded");
		this->SetMesh(mesh);
		return;
	}

	auto meshPtr = mesh.GetMesh().lock();
	size_t subCount = meshPtr->GetSubMeshes().size();
	Logger::Log("Button::Start: quad mesh submeshes:", subCount);

	// Create a per-button unlit material so the tint/color is unique per button
	ID3D11Device* dev = AssetManager::GetInstance().GetDevicePointer();
	if (!dev) {
		Logger::Warn("Button::Start: D3D11 device not available yet, skipping per-button material");
		this->SetMesh(mesh);
		return;
	}

	if (subCount == 0) {
		Logger::Warn("Button::Start: quad mesh has no submeshes, cannot assign material");
		this->SetMesh(mesh);
		return;
	}

	auto mat = std::make_shared<UnlitMaterial>(dev);
	mat->unlitShader = AssetManager::GetInstance().GetShaderPtr("PSUnlit");
	mat->diffuseTexture = nullptr;
	mat->color[0] = this->color.x;
	mat->color[1] = this->color.y;
	mat->color[2] = this->color.z;
	mat->color[3] = this->color.w;

	// Register material in AssetManager so it is owned and doesn't expire
	std::string matId = std::string("buttonMat_") + std::to_string(reinterpret_cast<uintptr_t>(this));
	AssetManager::GetInstance().AddMaterial(matId, mat);

	mesh.SetMaterial(0, mat);
	// keep a shared_ptr to the material so we can update it later
	this->material = mat;
	Logger::Log("Button::Start: Assigned per-button material to quad mesh (", matId, ")");

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

	// Color
	float col[4] = {this->color.x, this->color.y, this->color.z, this->color.w};
	if (ImGui::ColorEdit4("Color", col)) {
		this->color.x = col[0];
		this->color.y = col[1];
		this->color.z = col[2];
		this->color.w = col[3];

		// Keep material in sync if assigned
		if (this->material) {
			this->material->color[0] = this->color.x;
			this->material->color[1] = this->color.y;
			this->material->color[2] = this->color.z;
			this->material->color[3] = this->color.w;
		}
	}

	ImGui::Separator();
}

void UI::Button::SetLabel(const std::string& l) { this->label = l; }

std::string UI::Button::GetLabel() const { return this->label; }

void UI::Button::LoadFromJson(const nlohmann::json& data) {
	// Load base widget fields
	this->Widget::LoadFromJson(data);

	if (data.contains("label")) this->SetLabel(data.at("label").get<std::string>());
	if (data.contains("color") && data["color"].is_array() && data["color"].size() == 4) {
		this->color.x = data["color"][0].get<float>();
		this->color.y = data["color"][1].get<float>();
		this->color.z = data["color"][2].get<float>();
		this->color.w = data["color"][3].get<float>();
	}
}

void UI::Button::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["type"] = "UI::Button";
	data["label"] = this->GetLabel();
	data["color"] = {this->color.x, this->color.y, this->color.z, this->color.w};
}

void UI::Button::SetTint(const DirectX::XMFLOAT4& c) {
	this->color = c;
	if (this->material) {
		this->material->color[0] = c.x;
		this->material->color[1] = c.y;
		this->material->color[2] = c.z;
		this->material->color[3] = c.w;
	}
}

DirectX::XMFLOAT4 UI::Button::GetTint() const { return this->color; }
