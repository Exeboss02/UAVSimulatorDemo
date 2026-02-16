#include "UI/canvas.h"

#include <algorithm>

void UI::Canvas::SetSize(Vec2 size) { this->size = size; }

UI::Vec2 UI::Canvas::GetSize() const { return this->size; }

void UI::Canvas::AddChild(const std::shared_ptr<Widget>& child) {
	if (!child) return;
	this->children.push_back(child);
}

void UI::Canvas::RemoveChild(const std::shared_ptr<Widget>& child) {
	this->children.erase(std::remove(this->children.begin(), this->children.end(), child), this->children.end());
}

void UI::Canvas::Clear() { this->children.clear(); }

const std::vector<std::shared_ptr<UI::Widget>>& UI::Canvas::GetChildren() const { return this->children; }

void UI::Canvas::Update(float dt) {
	for (auto& child : children) {
		if (child && child->isEnabled()) child->Update(dt);
	}
}

void UI::Canvas::Draw() {
	for (auto& child : children) {
		if (child && child->IsVisible()) child->Draw();
	}
}

std::shared_ptr<UI::Widget> UI::Canvas::HitTest(Vec2 point) const {
	for (auto it = this->children.rbegin(); it != this->children.rend(); ++it) {
		if (!*it || !(*it)->IsVisible() || !(*it)->isEnabled()) continue;
		auto hit = HitTestRecursive(*it, point);
		if (hit) return hit;
	}

	return nullptr;
}

std::shared_ptr<UI::Widget> UI::Canvas::HitTestRecursive(const std::shared_ptr<Widget>& node, Vec2 point) const {
	if (node->HitTest(point)) return node;
	return nullptr;
}