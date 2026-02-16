#include "UI/widget.h"

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