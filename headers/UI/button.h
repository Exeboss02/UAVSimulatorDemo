#pragma once

#include "UI/widget.h"

// std
#include <DirectXMath.h>
#include <functional>
#include <memory>

// Forward-declare material to avoid heavy include in header
class UnlitMaterial;

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

	// Event ID helpers
	void SetOnClickEventID(int id);
	int GetOnClickEventID() const;
	void SetOnPressedEventID(int id);
	int GetOnPressedEventID() const;
	void SetOnReleasedEventID(int id);
	int GetOnReleasedEventID() const;
	void SetOnHoverEventID(int id);
	int GetOnHoverEventID() const;

	// Wired setters: set the ID and attempt to wire callbacks immediately
	void SetOnClickEventID_Wire(int id);
	void SetOnPressedEventID_Wire(int id);
	void SetOnReleasedEventID_Wire(int id);
	void SetOnHoverEventID_Wire(int id);

	bool IsHovered() const;
	bool isPressed() const;
	// Serialization helpers
	void SetLabel(const std::string& l);
	std::string GetLabel() const;

	void SetTint(const DirectX::XMFLOAT4& c);
	DirectX::XMFLOAT4 GetTint() const;

	// Serialization
	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	// Draw override to render label text
	void Draw() override;

	enum class HorizontalAlign { LEFT = 0, CENTER = 1, RIGHT = 2 };
	enum class VerticalAlign { TOP = 0, MIDDLE = 1, BOTTOM = 2 };

	void SetHorizontalAlign(HorizontalAlign a) { this->hAlign = a; }
	HorizontalAlign GetHorizontalAlign() const { return this->hAlign; }

	void SetVerticalAlign(VerticalAlign a) { this->vAlign = a; }
	VerticalAlign GetVerticalAlign() const { return this->vAlign; }

private:
	bool hovered = false;
	bool pressed = false;

	std::string label;

	std::function<void()> onClick;
	std::function<void()> onPressed;
	std::function<void()> onReleased;
	std::function<void()> onHover;
	std::function<void()> onUnhover;

	// Optional event IDs for wiring to EventManager (0 = none)
	int onClickEventID = 0;
	int onPressedEventID = 0;
	int onReleasedEventID = 0;
	int onHoverEventID = 0;

	DirectX::XMFLOAT4 color{1.0f, 1.0f, 1.0f, 1.0f};

	// Per-button material (kept alive by AssetManager as well)
	std::shared_ptr<UnlitMaterial> material;

	// Label alignment defaults
	HorizontalAlign hAlign = HorizontalAlign::CENTER;
	VerticalAlign vAlign = VerticalAlign::MIDDLE;
};

} // namespace UI