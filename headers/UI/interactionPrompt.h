#pragma once

#include "UI/canvasObject.h"
#include <DirectXMath.h>
#include <string>

namespace UI {
class Text;

class InteractionPrompt : public CanvasObject {
public:
	InteractionPrompt() = default;
	~InteractionPrompt() override = default;

	void Start() override;

	// Show prompt with text at world position
	void Show(const std::string& text, DirectX::XMVECTOR worldPos);
	void Hide();

	void SetOffset(float x, float y) {
		this->offsetX = x;
		this->offsetY = y;
	}
	void SetFontSize(float s) { this->fontSize = s; }

private:
	std::weak_ptr<Text> textWidget;
	float offsetX = 0.0f;
	float offsetY = -50.0f;
	float fontSize = 18.0f;
};

} // namespace UI
