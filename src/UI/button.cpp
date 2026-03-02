#include "UI/button.h"

#include "UI/canvasObject.h"
#include "core/assetManager.h"
#include "core/input/inputManager.h"
#include "rendering/texture.h"
#include "rendering/unlitMaterial.h"
#include "utilities/logger.h"

#include "UI/textRenderer.h"
#include "scene/sceneManager.h"

#include <DirectXMath.h>

// json
#include <nlohmann/json.hpp>

// std
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

	this->SetMesh(mesh);
}

void UI::Button::Update(float dt) {
	if (!this->visible || !this->enabled) return;

	auto mp = InputManager::GetInstance().GetMousePosition();
	int mx = mp[0];
	int my = mp[1];
	Vec2 point{static_cast<float>(mx), static_cast<float>(my)};
	const bool hit = this->HitTest(point);

	// Determine top-most widget at this point via the parent CanvasObject (if any).
	std::shared_ptr<UI::Widget> topMost = nullptr;
	auto parent = this->GetParent();
	if (!parent.expired()) {
		auto p = parent.lock();
		// Walk up until we find a CanvasObject
		GameObject* current = p.get();
		while (current) {
			if (auto canvasObj = dynamic_cast<UI::CanvasObject*>(current)) {
				topMost = canvasObj->HitTest(point);
				break;
			}
			auto nextParent = current->GetParent();
			if (nextParent.expired()) break;
			current = nextParent.lock().get();
		}
	}

	bool isTop = false;
	if (topMost) {
		if (this->GetPtr()) {
			auto self = std::static_pointer_cast<UI::Widget>(this->GetPtr());
			isTop = (topMost == self);
		}
	} else {
		// No canvas found; fallback to per-widget hit behaviour
		isTop = true;
	}

	bool effectiveHit = hit && isTop;

	if (effectiveHit && !this->hovered) {
		this->hovered = true;
		if (this->onHover) this->onHover();
	} else if (!effectiveHit && this->hovered) {
		this->hovered = false;
		if (this->onUnhover) this->onUnhover();
	}

	if (effectiveHit && InputManager::GetInstance().WasLMPressed()) {
		this->pressed = true;
		if (this->onPressed) this->onPressed();
	}

	if (effectiveHit && InputManager::GetInstance().WasLMReleased()) {
		this->pressed = false;
		if (this->onReleased) this->onReleased();
		if (effectiveHit && this->onClick) this->onClick();
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

	// Event IDs (editable in the editor). Use 0 for none.
	int clickId = this->onClickEventID;
	if (ImGui::InputInt("OnClick Event ID", &clickId)) {
		SetOnClickEventID_Wire(clickId);
	}

	int pressedId = this->onPressedEventID;
	if (ImGui::InputInt("OnPressed Event ID", &pressedId)) {
		SetOnPressedEventID_Wire(pressedId);
	}

	int releasedId = this->onReleasedEventID;
	if (ImGui::InputInt("OnReleased Event ID", &releasedId)) {
		SetOnReleasedEventID_Wire(releasedId);
	}

	ImGui::Separator();
	// Alignment controls
	ImGui::Text("Label alignment:");
	// Horizontal
	const char* hItems[] = {"Left", "Center", "Right"};
	int hCur = static_cast<int>(this->GetHorizontalAlign());
	if (ImGui::Combo("Horizontal", &hCur, hItems, IM_ARRAYSIZE(hItems))) {
		this->SetHorizontalAlign(static_cast<Button::HorizontalAlign>(hCur));
	}
	// Vertical
	const char* vItems[] = {"Top", "Middle", "Bottom"};
	int vCur = static_cast<int>(this->GetVerticalAlign());
	if (ImGui::Combo("Vertical", &vCur, vItems, IM_ARRAYSIZE(vItems))) {
		this->SetVerticalAlign(static_cast<Button::VerticalAlign>(vCur));
	}
}

void UI::Button::SetOnClickEventID(int id) { this->onClickEventID = id; }

int UI::Button::GetOnClickEventID() const { return this->onClickEventID; }

void UI::Button::SetOnPressedEventID(int id) { this->onPressedEventID = id; }

int UI::Button::GetOnPressedEventID() const { return this->onPressedEventID; }

void UI::Button::SetOnReleasedEventID(int id) { this->onReleasedEventID = id; }

int UI::Button::GetOnReleasedEventID() const { return this->onReleasedEventID; }

// After changing the stored event IDs, attempt to wire the button to the active SceneManager's EventManager
// so edits in the inspector take effect immediately.
void UI::Button::SetOnClickEventID_Wire(int id) {
	this->SetOnClickEventID(id);
	if (auto sm = SceneManager::GetActive()) sm->WireButtonEvents(this);
}

void UI::Button::SetOnPressedEventID_Wire(int id) {
	this->SetOnPressedEventID(id);
	if (auto sm = SceneManager::GetActive()) sm->WireButtonEvents(this);
}

void UI::Button::SetOnReleasedEventID_Wire(int id) {
	this->SetOnReleasedEventID(id);
	if (auto sm = SceneManager::GetActive()) sm->WireButtonEvents(this);
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

	// Load persisted event IDs if present
	if (data.contains("onClickEvent") && data["onClickEvent"].is_number())
		this->onClickEventID = data["onClickEvent"].get<int>();
	if (data.contains("onPressedEvent") && data["onPressedEvent"].is_number())
		this->onPressedEventID = data["onPressedEvent"].get<int>();
	if (data.contains("onReleasedEvent") && data["onReleasedEvent"].is_number())
		this->onReleasedEventID = data["onReleasedEvent"].get<int>();

	// Alignment
	if (data.contains("hAlign") && data["hAlign"].is_string()) {
		std::string s = data["hAlign"].get<std::string>();
		if (s == "left")
			this->hAlign = Button::HorizontalAlign::LEFT;
		else if (s == "center")
			this->hAlign = Button::HorizontalAlign::CENTER;
		else if (s == "right")
			this->hAlign = Button::HorizontalAlign::RIGHT;
	}
	if (data.contains("vAlign") && data["vAlign"].is_string()) {
		std::string s = data["vAlign"].get<std::string>();
		if (s == "top")
			this->vAlign = Button::VerticalAlign::TOP;
		else if (s == "middle")
			this->vAlign = Button::VerticalAlign::MIDDLE;
		else if (s == "bottom")
			this->vAlign = Button::VerticalAlign::BOTTOM;
	}
}

void UI::Button::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["type"] = "UI::Button";
	data["label"] = this->GetLabel();
	data["color"] = {this->color.x, this->color.y, this->color.z, this->color.w};

	// Persist event IDs
	if (this->onClickEventID != 0) data["onClickEvent"] = this->onClickEventID;
	if (this->onPressedEventID != 0) data["onPressedEvent"] = this->onPressedEventID;
	if (this->onReleasedEventID != 0) data["onReleasedEvent"] = this->onReleasedEventID;

	// Alignment
	switch (this->hAlign) {
	case Button::HorizontalAlign::LEFT:
		data["hAlign"] = "left";
		break;
	case Button::HorizontalAlign::CENTER:
		data["hAlign"] = "center";
		break;
	case Button::HorizontalAlign::RIGHT:
		data["hAlign"] = "right";
		break;
	}

	switch (this->vAlign) {
	case Button::VerticalAlign::TOP:
		data["vAlign"] = "top";
		break;
	case Button::VerticalAlign::MIDDLE:
		data["vAlign"] = "middle";
		break;
	case Button::VerticalAlign::BOTTOM:
		data["vAlign"] = "bottom";
		break;
	}
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

void UI::Button::Draw() {
	if (!this->IsVisible()) return;

	// Draw children first
	Widget::Draw();

	// Render label text centered inside the button
	if (this->label.empty()) return;

	// Choose font size relative to button height so measured width matches render size.
	UI::Vec2 pos = this->GetPosition();
	UI::Vec2 sz = this->GetSize();

	float fontSize = std::max(10.0f, sz.y * 0.6f);
	std::string font = "assets/fonts/lucon.ttf";
	float measured = UI::TextRenderer::GetInstance().MeasureString(this->label, fontSize, font).x;

	// Horizontal alignment
	float paddingX = 8.0f;
	float x = pos.x + paddingX; // default left
	switch (this->hAlign) {
	case Button::HorizontalAlign::LEFT:
		x = pos.x + paddingX;
		break;
	case Button::HorizontalAlign::CENTER: {
		float centerX = pos.x + sz.x * 0.5f;
		x = centerX - measured * 0.5f; // midpoint of string at button center
		break;
	}
	case Button::HorizontalAlign::RIGHT:
		x = pos.x + sz.x - measured - paddingX;
		break;
	}

	// Vertical alignment
	float paddingY = 4.0f;
	float y = pos.y + paddingY; // top
	switch (this->vAlign) {
	case Button::VerticalAlign::TOP:
		y = pos.y + paddingY;
		break;
	case Button::VerticalAlign::MIDDLE:
		y = pos.y + (sz.y - fontSize) * 0.5f;
		break;
	case Button::VerticalAlign::BOTTOM:
		y = pos.y + sz.y - fontSize - paddingY;
		break;
	}

	// Pick contrasting text color against button tint
	float r = this->color.x;
	float g = this->color.y;
	float b = this->color.z;
	float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
	DirectX::XMFLOAT4 textCol =
		(lum > 0.6f) ? DirectX::XMFLOAT4{0.0f, 0.0f, 0.0f, 1.0f} : DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f};

	UI::TextRenderer::GetInstance().SubmitText(this->label, UI::Vec2{x, y}, fontSize, textCol, font,
											   this->GetZIndex() + 1);
}
