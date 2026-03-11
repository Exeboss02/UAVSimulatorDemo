#pragma once

#include "UI/scrollableWidget.h"

// std
#include <DirectXMath.h>
#include <string>
#include <vector>

namespace UI {
class Image : public ScrollableWidget {
public:
	Image() = default;
	virtual ~Image() = default;

	virtual void Start() override {}
	virtual void ShowInHierarchy() override;

	// Serialization
	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	void Update(float dt) override;
	void Draw() override;

	void SetImage(const std::string& image);
	std::string GetImage() const;
	void SetTint(const DirectX::XMFLOAT4& c);
	DirectX::XMFLOAT4 GetTint() const;

private:
	// Path to image
	std::string image;

	// Edit buffer for ImGui input
	std::vector<char> imageEditBuffer;

	// Tint/color applied when rendering the image
	DirectX::XMFLOAT4 tint{1.0f, 1.0f, 1.0f, 1.0f};
};

} // namespace UI
