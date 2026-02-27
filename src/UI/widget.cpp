#include "UI/widget.h"
#include "UI/canvasObject.h"

// dx11
#include <DirectXMath.h>
#include <nlohmann/json.hpp>

UI::Widget::Widget(MeshObjData& mesh) { SetMesh(mesh); }

void UI::Widget::SetPosition(Vec2 pos) { 
	this->position = pos; 
	this->CalculateTruePosition();
}

void UI::Widget::SetSize(Vec2 size) { 
	this->size = size; 
	this->CalculateTruePosition();
}

UI::Vec2 UI::Widget::GetPosition() const { return this->truePosition; }

UI::Vec2 UI::Widget::GetSize() const { return this->size; }

void UI::Widget::SetAnchor(Anchor anchor) { this->anchor = anchor; }

Anchor UI::Widget::GetAnchor() const { return this->anchor; }

void UI::Widget::SetVisible(bool visible) { this->visible = visible; }

bool UI::Widget::IsVisible() const { return this->visible; }

void UI::Widget::SetEnabled(bool enabled) { this->enabled = enabled; }

bool UI::Widget::isEnabled() const { return this->enabled; }

void UI::Widget::Update(float dt) {
	using namespace DirectX;

	uintptr_t addr = reinterpret_cast<uintptr_t>(this);
	float epsilon = static_cast<float>(addr % 1000u) * 1e-6f; // range ~0..0.000999
	float z = 0.5f + static_cast<float>(this->GetZIndex()) * 0.001f + epsilon;
	XMVECTOR posVec =
		XMVectorSet(this->position.x + this->size.x * 0.5f, this->position.y + this->size.y * 0.5f, z, 1.0f);
	XMVECTOR scaleVec = XMVectorSet(this->size.x * 0.5f, this->size.y * 0.5f, 1.0f, 0.0f);
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

int UI::Widget::GetZIndex() const { return this->zIndex; }

void UI::Widget::SetZIndex(int z) { this->zIndex = z; }

void UI::Widget::SetCanvasSize(Vec2 size) { 
	this->canvasSize = size; 
	this->CalculateTruePosition();
}

void UI::Widget::CalculateTruePosition() {
	switch (this->anchor) {
		case (Anchor::TopLeft):
			this->truePosition = this->position;
			break;
		case (Anchor::TopRight):
			this->truePosition = Vec2(this->position.x + this->canvasSize.x - this->GetSize().x, this->position.y);
			break;
		case (Anchor::TopCenter):
			this->truePosition = Vec2(this->position.x + this->canvasSize.x / 2 - this->GetSize().x / 2, this->position.y);
			break;
		case (Anchor::BottomLeft):
			this->truePosition = Vec2(this->position.x, this->position.y + this->canvasSize.y);
			break;
		case (Anchor::BottomRight):
			this->truePosition =
				Vec2(this->position.x + this->canvasSize.x - this->GetSize().x, this->position.y + this->canvasSize.y);
			break;
		case (Anchor::BottomCenter):
			this->truePosition = Vec2(this->position.x + this->canvasSize.x / 2 - this->GetSize().x / 2,
									  this->position.y + this->canvasSize.y);
			break;
	}
	this->position;
}

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

	// Z-index
	int z = this->GetZIndex();
	if (ImGui::InputInt("Z Index", &z)) {
		this->SetZIndex(z);
	}

	ImGui::Separator();
}

void UI::Widget::OnDestroy() {
	Logger::Log("Widget: Trying to destroy object");

	// If this widget's parent is a CanvasObject, remove it from the canvas children list
	auto parent = this->GetParent();
	if (!parent.expired()) {
		auto p = parent.lock();
		if (p) {
			if (auto canvasObj = dynamic_cast<UI::CanvasObject*>(p.get())) {
				// Ensure GameObject parent-child bookkeeping is updated first
				this->SetParent(std::weak_ptr<GameObject>());

				// Cast our shared ptr to Widget and remove from canvas
				auto self = std::static_pointer_cast<UI::Widget>(this->GetPtr());
				canvasObj->RemoveChild(self);
				Logger::Log("Widget: Successfully removed itself from canvas");

				// Also remove from renderer UI queue to avoid stale weak_ptrs
				if (this->GetPtr()) {
					Logger::Log("Widget: Adress -> ", this->GetPtr());
					RenderQueue::RemoveUIWidget(this->GetPtr());
					Logger::Log("Widget: Successfully removed own pointer");
				}
			}
		}
	} else {
		Logger::Error("Widget: Expired parent when trying to remove object");
	}

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

	// zIndex
	if (data.contains("zIndex")) {
		this->SetZIndex(data.at("zIndex").get<int>());
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

	// zIndex
	data["zIndex"] = this->GetZIndex();
}