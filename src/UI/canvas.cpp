#include "UI/canvas.h"
#include "core/window.h"

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
	UINT w = Window::GetCurrentWidth();
	UINT h = Window::GetCurrentHeight();

	if (w > 0 && h > 0) {
		UI::Vec2 newSize{static_cast<float>(w), static_cast<float>(h)};
		if (newSize.x != this->size.x || newSize.y != this->size.y) {
			this->SetSize(newSize);
		}
	}

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
	// Consider z-index: test top-most first (higher z drawn on top)
	std::vector<std::shared_ptr<Widget>> live;
	live.reserve(this->children.size());
	for (const auto& c : this->children) {
		if (!c || !c->IsVisible() || !c->isEnabled()) continue;
		live.push_back(c);
	}

	std::sort(live.begin(), live.end(), [](const std::shared_ptr<Widget>& a, const std::shared_ptr<Widget>& b) {
		return a->GetZIndex() > b->GetZIndex();
	});

	for (const auto& c : live) {
		auto hit = HitTestRecursive(c, point);
		if (hit) return hit;
	}

	return nullptr;
}

std::shared_ptr<UI::Widget> UI::Canvas::HitTestRecursive(const std::shared_ptr<Widget>& node, Vec2 point) const {
	if (node->HitTest(point)) return node;
	return nullptr;
}