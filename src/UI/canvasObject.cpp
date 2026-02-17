#include "UI/canvasObject.h"
#include "UI/button.h"
#include "UI/canvas.h"
#include "UI/text.h"
#include "gameObjects/cameraObject.h"
#include "rendering/renderQueue.h"

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

	if (this->canvas) {
		auto sz = this->canvas->GetSize();
		if (sz.x == 0.0f || sz.y == 0.0f) {
			try {
				auto& cam = CameraObject::GetMainCamera();
				float aspect = cam.GetAspectRatio();
				float height = 1080.0f;
				float width = aspect * height;
				this->canvas->SetSize(UI::Vec2{width, height});
			} catch (...) {
				this->canvas->SetSize(UI::Vec2{1920.0f, 1080.0f});
			}
		}
	}

	this->SetName("Canvas");

	for (const auto& w : this->GetChildren()) {
		if (w.expired()) continue;
		auto child = w.lock();
		if (!child) continue;
		if (auto widget = std::dynamic_pointer_cast<UI::Widget>(child)) {
			this->AddChild(widget);
		}
	}
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
					newWidget->SetName("Button");
					newWidget->SetPosition({200, 200});
					newWidget->SetSize({100, 20});
					std::weak_ptr<GameObject> me = this->GetPtr();
					newWidget->SetParent(me);
					this->AddChild(std::static_pointer_cast<UI::Widget>(newWidget));

					RenderQueue::AddUIWidget(newWidget);
				}
			}
		}
		if (ImGui::Selectable("Text")) {
			if (this->factory) {
				auto newWidgetWeak = this->factory->CreateGameObjectOfType<UI::Text>();
				if (!newWidgetWeak.expired()) {
					auto newWidget = newWidgetWeak.lock();
					newWidget->SetName("Text");
					newWidget->SetText("Text");
					newWidget->SetPosition({50, 50});
					newWidget->SetSize({100, 20});
					std::weak_ptr<GameObject> me = this->GetPtr();
					newWidget->SetParent(me);
					this->AddChild(std::static_pointer_cast<UI::Widget>(newWidget));

					RenderQueue::AddUIWidget(newWidget);
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

					RenderQueue::AddUIWidget(newWidget);
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
	if (c.contains("scaleMode") && c["scaleMode"].is_string() && this->canvas) {
		std::string m = c["scaleMode"].get<std::string>();
		if (m == "stretch")
			this->canvas->SetScaleMode(Canvas::ScaleMode::Stretch);
		else
			this->canvas->SetScaleMode(Canvas::ScaleMode::Pixel);
	}

	// Widgets: legacy support only — if the scene loader already created child GameObjects
	// we must not create duplicates. Only create widgets from the "widgets" array when
	// there are no existing children for this CanvasObject.
	if (data.contains("widgets") && data["widgets"].is_array() && this->factory && this->GetChildren().empty()) {
		for (const auto& w : data["widgets"]) {
			if (!w.contains("type")) continue;
			std::string type = w.at("type").get<std::string>();

			std::weak_ptr<GameObject> createdWeak;
			if (type == "Button") {
				createdWeak = this->factory->CreateGameObjectOfType<UI::Button>();
			} else if (type == "Text") {
				createdWeak = this->factory->CreateGameObjectOfType<UI::Text>();
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

				// Text specific
				if (type == "Text") {
					if (auto txt = std::dynamic_pointer_cast<UI::Text>(widget)) {
						if (w.contains("text")) txt->SetText(w.at("text").get<std::string>());
						if (w.contains("font")) txt->SetFont(w.at("font").get<std::string>());
						if (w.contains("color") && w["color"].is_array() && w["color"].size() == 4) {
							DirectX::XMFLOAT4 col{w["color"][0].get<float>(), w["color"][1].get<float>(),
												  w["color"][2].get<float>(), w["color"][3].get<float>()};
							txt->SetColor(col);
						}
					}
				}

				// Add to canvas
				this->AddChild(widget);
				// Register in UI queue so UI pass will visit this widget
				RenderQueue::AddUIWidget(widget);
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
	data["canvas"]["scaleMode"] = (c->GetScaleMode() == Canvas::ScaleMode::Stretch) ? "stretch" : "pixel";

	// Note: widgets are saved as separate GameObject entries by the scene system.
	// Do not serialize a separate "widgets" array here to avoid duplication on load.
}
