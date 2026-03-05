#include "game/crosshair.h"
#include "UI/canvas.h"
#include "core/tools.h"
#include "gameObjects/cameraObject.h"
#include "rendering/renderQueue.h"

#include <DirectXMath.h>
#include <nlohmann/json.hpp>

void Crosshair::Start() {
	// Keep default behavior
	this->SetAnchor(UI::Anchor::MidCenter);
	if (this->GetImage().empty()) {
		this->SetImage(this->defaultImagePath);
	} else {
		this->defaultImagePath = this->GetImage();
	}
	this->defaultTint = this->GetTint();
	this->UI::Image::Start();
}

void Crosshair::Update(float dt) {
	float effectiveDt = dt;
	if (effectiveDt <= 0.0f) {
		effectiveDt = Time::GetInstance().GetDeltaTime();
	}

	if (this->hitIndicatorTimer > 0.0f) {
		this->hitIndicatorTimer -= effectiveDt;
		if (this->hitIndicatorTimer <= 0.0f) {
			this->SetImage(this->defaultImagePath);
			this->SetTint(this->defaultTint);
		}
	}

	this->UI::Image::Update(effectiveDt);
}

void Crosshair::ShowHitIndicator(float durationSeconds) {
	if (!this->GetImage().empty() && this->GetImage() != this->hitImagePath) {
		this->defaultImagePath = this->GetImage();
		this->defaultTint = this->GetTint();
	}

	this->SetImage(this->hitImagePath);
	this->SetTint(this->hitTint);
	this->hitIndicatorTimer = durationSeconds;
}

void Crosshair::ShowInHierarchy() {
	this->UI::Image::ShowInHierarchy();

	if (!DISABLE_IMGUI) {
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
}
