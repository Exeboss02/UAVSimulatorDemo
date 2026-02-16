#include "UI/canvasObject.h"
#include "UI/canvas.h"

namespace UI {

CanvasObject::CanvasObject(const std::shared_ptr<Canvas>& canvas) : canvas(canvas) {}

void CanvasObject::SetCanvas(const std::shared_ptr<Canvas>& canvas) { this->canvas = canvas; }

std::shared_ptr<Canvas> CanvasObject::GetCanvas() const { return this->canvas; }

bool CanvasObject::HasCanvas() const { return static_cast<bool>(this->canvas); }

void CanvasObject::AddChild(const std::shared_ptr<Widget>& child) {
	if (this->canvas) this->canvas->AddChild(child);
}

void CanvasObject::RemoveChild(const std::shared_ptr<Widget>& child) {
	if (this->canvas) this->canvas->RemoveChild(child);
}

void CanvasObject::Clear() {
	if (this->canvas) this->canvas->Clear();
}

void CanvasObject::Update(float dt) {
	if (this->canvas) this->canvas->Update(dt);
}

void CanvasObject::Draw() {
	if (this->canvas) this->canvas->Draw();
}

std::shared_ptr<Widget> CanvasObject::HitTest(Vec2 point) const {
	if (this->canvas) return this->canvas->HitTest(point);
	return nullptr;
}

void CanvasObject::Start() {
	// Ensure we own a Canvas when the GameObject is started
	if (!this->HasCanvas()) {
		this->SetCanvas(std::make_shared<Canvas>());
	}

	// Optionally initialize or set a name
	this->SetName("UI Canvas");
}

void CanvasObject::Tick() {
	// No dt available in GameObject::Tick; pass 0.0f
	if (this->canvas) this->canvas->Update(0.0f);
}

void CanvasObject::OnDestroy() {
	// Clean up canvas children when the GameObject is destroyed
	if (this->canvas) this->canvas->Clear();
}

} // namespace UI

void UI::CanvasObject::ShowInHierarchy() {
	// Show the common GameObject UI first
	GameObject::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Canvas options:");

	if (!this->HasCanvas()) {
		ImGui::Text("No Canvas assigned.");
		return;
	}

	auto c = this->GetCanvas();
	if (!c) return;

	// Size
	UI::Vec2 size = c->GetSize();
	float sz[2] = {size.x, size.y};
	if (ImGui::InputFloat2("Size", sz)) {
		UI::Vec2 newSize{sz[0], sz[1]};
		c->SetSize(newSize);
	}

	// Children
	const auto& children = c->GetChildren();
	ImGui::Text("Children: %d", static_cast<int>(children.size()));
	if (!children.empty()) {
		for (size_t i = 0; i < children.size(); ++i) {
			auto& child = children[i];
			if (!child) continue;

			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::TextUnformatted(child->GetName().c_str());
			ImGui::SameLine();
			if (ImGui::Button((std::string("Select##canvas_child_") + std::to_string(i)).c_str())) {
				std::weak_ptr<GameObject> w = std::static_pointer_cast<GameObject>(child);
				this->factory->SetSelected(w);
			}
		}
	}

	if (ImGui::Button("Clear Canvas Children")) {
		c->Clear();
	}
}
