#include "UI/widget.h"
#include "UI/canvasObject.h"

// dx11
#include <DirectXMath.h>
#include <nlohmann/json.hpp>

UI::Widget::Widget(MeshObjData& mesh) { SetMesh(mesh); }

void UI::Widget::SetPosition(Vec2 pos) { this->position = pos; }

void UI::Widget::SetSize(Vec2 size) { this->size = size; }

UI::Vec2 UI::Widget::GetPosition() const { return this->position; }

UI::Vec2 UI::Widget::GetSize() const { return this->size; }

void UI::Widget::SetVisible(bool visible) { this->visible = visible; }

bool UI::Widget::IsVisible() const { return this->visible; }

void UI::Widget::SetEnabled(bool enabled) { this->enabled = enabled; }

bool UI::Widget::isEnabled() const { return this->enabled; }

void UI::Widget::Update(float dt) {
	// Map widget screen-space position/size into the GameObject3D transform
	// so the renderer can draw it with an orthographic camera.
	using namespace DirectX;
	XMVECTOR posVec =
		XMVectorSet(this->position.x + this->size.x * 0.5f, this->position.y + this->size.y * 0.5f, 0.5f, 1.0f);
	XMVECTOR scaleVec = XMVectorSet(this->size.x, this->size.y, 1.0f, 0.0f);
	this->transform.SetPosition(posVec);
	this->transform.SetScale(scaleVec);

	for (auto& child : children) {
		if (child && child->enabled) child->Update(dt);
	}
}

void UI::Widget::Draw() {
	if (!this->visible) return;
	for (auto& child : children) {
		if (child && child->visible) child->Draw();
	}
}

bool UI::Widget::HitTest(Vec2 point) const {
	return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y &&
		   point.y <= position.y + size.y;
}

void UI::Widget::SetWidgetMesh(MeshObjData& mesh) { SetMesh(mesh); }

void UI::Widget::ShowInHierarchy() {
	// First display common GameObject inspector
	GameObject::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Widget settings:");

	// Position
	Vec2 pos = this->GetPosition();
	if (ImGui::InputFloat2("Position", &pos.x)) {
		this->SetPosition(pos);
	}

	// Size
	Vec2 size = this->GetSize();
	if (ImGui::InputFloat2("Size", &size.x)) {
		this->SetSize(size);
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

	ImGui::Separator();
}

void UI::Widget::OnDestroy() {
	// If this widget's parent is a CanvasObject, remove it from the canvas children list
	auto parent = this->GetParent();
	if (!parent.expired()) {
		auto p = parent.lock();
		if (p) {
			if (auto canvasObj = dynamic_cast<UI::CanvasObject*>(p.get())) {
				// Cast our shared ptr to Widget and remove from canvas
				auto self = std::static_pointer_cast<UI::Widget>(this->GetPtr());
				canvasObj->RemoveChild(self);
			}
		}
	}

	// Call base cleanup if any
	GameObject::OnDestroy();
}

void UI::Widget::LoadFromJson(const nlohmann::json& data) {
	// Base properties (name etc)
	this->GameObject::LoadFromJson(data);

	if (data.contains("visible")) this->SetVisible(data.at("visible").get<bool>());
	if (data.contains("enabled")) this->SetEnabled(data.at("enabled").get<bool>());

	if (data.contains("transform") && data["transform"].contains("position") && data["transform"].contains("size")) {
		auto pos = data["transform"]["position"];
		auto sz = data["transform"]["size"];
		if (pos.is_array() && pos.size() == 2 && sz.is_array() && sz.size() == 2) {
			UI::Vec2 p{pos[0].get<float>(), pos[1].get<float>()};
			UI::Vec2 s{sz[0].get<float>(), sz[1].get<float>()};
			this->SetPosition(p);
			this->SetSize(s);
		}
	}
}

void UI::Widget::SaveToJson(nlohmann::json& data) {
	// Write common GameObject fields (children, name)
	this->GameObject::SaveToJson(data);
	// Mark as UI::Widget so loader can construct the right type
	data["type"] = "UI::Widget";

	// Widget-specific fields
	data["visible"] = this->IsVisible();
	data["enabled"] = this->isEnabled();
	auto pos = this->GetPosition();
	auto sz = this->GetSize();
	data["transform"]["position"] = {pos.x, pos.y};
	data["transform"]["size"] = {sz.x, sz.y};
}