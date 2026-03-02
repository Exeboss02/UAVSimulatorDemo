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
	void ShowHitIndicator(float durationSeconds = 0.1f);

private:
	bool autoCenter = true;
	float hitIndicatorTimer = 0.0f;
	std::string defaultImagePath = "assets/images/crosshair.png";
	std::string hitImagePath = "assets/images/crosshair_hit.png";
	DirectX::XMFLOAT4 defaultTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT4 hitTint = DirectX::XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
};