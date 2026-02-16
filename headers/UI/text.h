#pragma once

#include "UI/widget.h"
#include <DirectXMath.h>
#include <string>

namespace UI {

class Text : public Widget {
public:
	Text() = default;
	virtual ~Text() = default;

	virtual void Start() override {}
	virtual void ShowInHierarchy() override;

	// Serialization
	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	void SetText(const std::string& t);
	std::string GetText() const;

	void SetFont(const std::string& f);
	std::string GetFont() const;

	void SetColor(const DirectX::XMFLOAT4& c);
	DirectX::XMFLOAT4 GetColor() const;

private:
	std::string text;
	std::string font;
	DirectX::XMFLOAT4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

} // namespace UI
