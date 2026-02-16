#pragma once

#include "UI/widget.h"

// std
#include <functional>

namespace UI {
class Button : public Widget {
public:
	virtual void Start() override;

	// Show button-specific options
	void ShowInHierarchy() override;

	void Update(float dt) override;

	void SetOnClick(std::function<void()> callback);
	void SetOnPressed(std::function<void()> callback);
	void SetOnReleased(std::function<void()> callback);
	void SetOnHover(std::function<void()> callback);
	void SetOnUnhover(std::function<void()> callback);

	bool IsHovered() const;
	bool isPressed() const;
	// Serialization helpers
	void SetLabel(const std::string& l);
	std::string GetLabel() const;

private:
	bool hovered = false;
	bool pressed = false;

	std::string label;

	std::function<void()> onClick;
	std::function<void()> onPressed;
	std::function<void()> onReleased;
	std::function<void()> onHover;
	std::function<void()> onUnhover;
};

} // namespace UI