#include "game/crosshair.h"
#include "UI/canvas.h"
#include "gameObjects/cameraObject.h"
#include "rendering/renderQueue.h"

#include <DirectXMath.h>
#include <nlohmann/json.hpp>

void Crosshair::Start() {
	// Keep default behavior
	this->SetAnchor(UI::Anchor::MidCenter);
	this->UI::Image::Start();
}

void Crosshair::Update(float dt) {}

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
