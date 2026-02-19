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

private:
	bool hovered = false;
	bool pressed = false;

	std::string label;

	std::function<void()> onClick;
	std::function<void()> onPressed;
	std::function<void()> onReleased;
	std::function<void()> onHover;
	std::function<void()> onUnhover;

	DirectX::XMFLOAT4 color{1.0f, 1.0f, 1.0f, 1.0f};

	// Per-button material (kept alive by AssetManager as well)
	std::shared_ptr<UnlitMaterial> material;
};

} // namespace UI