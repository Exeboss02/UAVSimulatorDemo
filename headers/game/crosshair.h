#pragma once

#include "UI/canvasObject.h"
#include "UI/image.h"
#include "gameObjects/cameraObject.h"

class Crosshair : public UI::Image {
public:
	Crosshair() = default;
	~Crosshair() = default;

	void Start() override;
	void Update(float dt) override;
	void ShowInHierarchy() override;

	// Convenience setters
	void SetCrosshairImage(const std::string& img) { this->SetImage(img); }
	void SetCrosshairSize(float w, float h) { this->SetSize(UI::Vec2{w, h}); }

private:
	bool autoCenter = true;
};