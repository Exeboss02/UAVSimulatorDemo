#include "game/crosshair.h"
#include "UI/canvas.h"
#include "gameObjects/cameraObject.h"
#include "rendering/renderQueue.h"

#include <DirectXMath.h>
#include <nlohmann/json.hpp>

void Crosshair::Start() {
	// Keep default behavior
	this->UI::Image::Start();
}

void Crosshair::Update(float dt) {
	// If auto-center is enabled, place the widget in the center of the canvas (or fallback to camera/aspect)
	if (this->autoCenter) {
		// Try to find a parent CanvasObject
		try {
			auto parentWeak = this->GetParent();
			if (!parentWeak.expired()) {
				auto parent = parentWeak.lock();
				if (parent) {
					if (auto canvasObj = dynamic_cast<UI::CanvasObject*>(parent.get())) {
						auto canvas = canvasObj->GetCanvas();
						if (canvas) {
							auto sz = canvas->GetSize();
							auto s = this->GetSize();
							UI::Vec2 pos{(sz.x - s.x) * 0.5f, (sz.y - s.y) * 0.5f};
							this->SetPosition(pos);
						}
					}
				}
			}
		} catch (const std::exception& e) {
			Logger::Error("Crosshair::Update parent canvas lookup failed: ", e.what());
			// Fallback: compute a reasonable canvas size from main camera aspect
			try {
				auto& cam = CameraObject::GetMainCamera();
				float aspect = cam.GetAspectRatio();
				float height = 1080.0f;
				float width = aspect * height;
				auto s = this->GetSize();
				UI::Vec2 pos{(width - s.x) * 0.5f, (height - s.y) * 0.5f};
				this->SetPosition(pos);
			} catch (const std::exception& e2) {
				Logger::Error("Crosshair::Update fallback failed: ", e2.what());
			} catch (...) {
				Logger::Error("Crosshair::Update fallback unknown exception");
			}
		} catch (...) {
			Logger::Error("Crosshair::Update unknown exception");
			// Fallback: compute a reasonable canvas size from main camera aspect
			try {
				auto& cam = CameraObject::GetMainCamera();
				float aspect = cam.GetAspectRatio();
				float height = 1080.0f;
				float width = aspect * height;
				auto s = this->GetSize();
				UI::Vec2 pos{(width - s.x) * 0.5f, (height - s.y) * 0.5f};
				this->SetPosition(pos);
			} catch (const std::exception& e2) {
				Logger::Error("Crosshair::Update fallback failed: ", e2.what());
			} catch (...) {
				Logger::Error("Crosshair::Update fallback unknown exception");
			}
		}
	}

	// Call base update so transform/children are updated
	this->UI::Image::Update(dt);
}

void Crosshair::ShowInHierarchy() {
	this->UI::Image::ShowInHierarchy();

	ImGui::Separator();
	ImGui::Text("Crosshair settings:");

	if (ImGui::Checkbox("Auto Center", &this->autoCenter)) {
		// no-op; Update() will apply
	}

	// Size control (width, height)
	auto s = this->GetSize();
	if (ImGui::InputFloat2("Crosshair Size", &s.x)) {
		this->SetSize(s);
	}

	ImGui::Separator();
}
