#include "UI/canvasObject.h"
#include "UI/button.h"
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
	if (!this->canvas) return;

	if (this->factory) {
		const auto& children = this->canvas->GetChildren();
		for (const auto& child : children) {
			if (!child) continue;
			auto goShared = std::static_pointer_cast<GameObject>(child);
			this->factory->QueueDeleteGameObject(goShared);
		}
	}

	if (this->canvas) {
		std::vector<std::shared_ptr<Widget>> toRemove = this->canvas->GetChildren();
		for (const auto& child : toRemove) {
			if (!child) continue;
			this->canvas->RemoveChild(child);
		}
	}
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

	if (!this->HasCanvas()) {
		this->SetCanvas(std::make_shared<Canvas>());
	}

	this->SetName("UI Canvas");
}

void CanvasObject::Tick() {
	if (this->canvas) this->canvas->Update(0.0f);
}

void CanvasObject::OnDestroy() {
	if (this->canvas && this->factory) {
		const auto& children = this->canvas->GetChildren();
		for (const auto& child : children) {
			if (!child) continue;
			std::weak_ptr<GameObject> go = std::static_pointer_cast<GameObject>(child);
			this->factory->QueueDeleteGameObject(go);
		}
	}
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

	auto canvas = this->GetCanvas();
	if (!canvas) return;

	// Size
	UI::Vec2 size = canvas->GetSize();
	if (ImGui::InputFloat2("Size", &size.x)) {
		canvas->SetSize(size);
	}

	// Children
	const auto& children = canvas->GetChildren();
	ImGui::Text("Children: %d", static_cast<int>(children.size()));
	ImGui::SameLine();
	if (ImGui::Button("Add Widget")) {
		ImGui::OpenPopup("add_widget_popup");
	}

	if (ImGui::BeginPopup("add_widget_popup")) {
		if (ImGui::Selectable("Button")) {
			if (this->factory) {
				auto newWidgetWeak = this->factory->CreateGameObjectOfType<UI::Button>();
				if (!newWidgetWeak.expired()) {
					auto newWidget = newWidgetWeak.lock();
					std::weak_ptr<GameObject> me = this->GetPtr();
					newWidget->SetParent(me);
					this->AddChild(std::static_pointer_cast<UI::Widget>(newWidget));
				}
			}
		}
		if (ImGui::Selectable("Text")) {
			if (this->factory) {
				auto newWidgetWeak = this->factory->CreateGameObjectOfType<UI::Widget>();
				if (!newWidgetWeak.expired()) {
					auto newWidget = newWidgetWeak.lock();
					newWidget->SetName("Text");
					std::weak_ptr<GameObject> me = this->GetPtr();
					newWidget->SetParent(me);
					this->AddChild(std::static_pointer_cast<UI::Widget>(newWidget));
				}
			}
		}
		if (ImGui::Selectable("Image")) {
			if (this->factory) {
				auto newWidgetWeak = this->factory->CreateGameObjectOfType<UI::Widget>();
				if (!newWidgetWeak.expired()) {
					auto newWidget = newWidgetWeak.lock();
					newWidget->SetName("Image");
					std::weak_ptr<GameObject> me = this->GetPtr();
					newWidget->SetParent(me);
					this->AddChild(std::static_pointer_cast<UI::Widget>(newWidget));
				}
			}
		}
		ImGui::EndPopup();
	}
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
		canvas->Clear();
	}
}

void UI::CanvasObject::LoadFromJson(const nlohmann::json& data) {
	// Base properties
	this->GameObject::LoadFromJson(data);

	if (!data.contains("canvas")) {
		Logger::Error("No canvas found in scene.");
		return;
	}

	const auto& c = data.at("canvas");
	if (c.contains("size") && c["size"].is_array() && c["size"].size() == 2) {
		UI::Vec2 s{};
		s.x = c["size"][0].get<float>();
		s.y = c["size"][1].get<float>();
		if (!this->HasCanvas()) this->SetCanvas(std::make_shared<Canvas>());
		if (this->canvas) this->canvas->SetSize(s);
	}

	// Widgets
	if (data.contains("widgets") && data["widgets"].is_array() && this->factory) {
		for (const auto& w : data["widgets"]) {
			if (!w.contains("type")) continue;
			std::string type = w.at("type").get<std::string>();

			std::weak_ptr<GameObject> createdWeak;
			if (type == "Button") {
				createdWeak = this->factory->CreateGameObjectOfType<UI::Button>();
			} else {
				createdWeak = this->factory->CreateGameObjectOfType<UI::Widget>();
			}

			if (createdWeak.expired()) continue;
			auto created = createdWeak.lock();

			// Parent to this CanvasObject so Scene hierarchy is correct
			created->SetParent(this->GetPtr());

			// Cast to widget to set widget-specific fields
			if (auto widget = std::dynamic_pointer_cast<UI::Widget>(created)) {
				// Name
				if (w.contains("name")) widget->SetName(w.at("name").get<std::string>());

				// Active/visible/enabled
				if (w.contains("active")) widget->SetActive(w.at("active").get<bool>());
				if (w.contains("visible")) widget->SetVisible(w.at("visible").get<bool>());
				if (w.contains("enabled")) widget->SetEnabled(w.at("enabled").get<bool>());

				// Transform
				if (w.contains("transform") && w["transform"].contains("position") && w["transform"].contains("size")) {
					auto pos = w["transform"]["position"];
					auto size = w["transform"]["size"];
					if (pos.is_array() && pos.size() == 2 && size.is_array() && size.size() == 2) {
						UI::Vec2 p{pos[0].get<float>(), pos[1].get<float>()};
						UI::Vec2 s{size[0].get<float>(), size[1].get<float>()};
						widget->SetPosition(p);
						widget->SetSize(s);
					}
				}

				// Button specific
				if (type == "Button") {
					if (auto btn = std::dynamic_pointer_cast<UI::Button>(widget)) {
						if (w.contains("label")) btn->SetLabel(w.at("label").get<std::string>());
					}
				}

				// Add to canvas
				this->AddChild(widget);
			}
		}
	}
}

void UI::CanvasObject::SaveToJson(nlohmann::json& data) {
	this->GameObject::SaveToJson(data);

	data["type"] = "UI::CanvasObject";

	if (!this->HasCanvas()) return;

	auto c = this->GetCanvas();
	if (!c) return;

	// Canvas
	auto size = c->GetSize();
	data["canvas"]["size"] = {size.x, size.y};

	// Widgets
	const auto& children = c->GetChildren();
	for (size_t i = 0; i < children.size(); ++i) {
		auto& child = children[i];
		if (!child) continue;

		nlohmann::json w;
		// Type
		if (dynamic_cast<UI::Button*>(child.get())) {
			w["type"] = "Button";
		} else {
			w["type"] = "Widget";
		}

		// Common fields
		w["name"] = child->GetName();
		w["active"] = child->IsActive();

		// Widget-specific properties
		if (auto widget = std::dynamic_pointer_cast<UI::Widget>(child)) {
			w["visible"] = widget->IsVisible();
			w["enabled"] = widget->isEnabled();
			auto pos = widget->GetPosition();
			auto sz = widget->GetSize();
			w["transform"]["position"] = {pos.x, pos.y};
			w["transform"]["size"] = {sz.x, sz.y};

			if (auto btn = std::dynamic_pointer_cast<UI::Button>(widget)) {
				w["label"] = btn->GetLabel();
			}
		}

		data["widgets"].push_back(w);
	}
}
