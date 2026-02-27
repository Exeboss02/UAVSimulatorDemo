#pragma once

#include "UI/widget.h"
#include <DirectXMath.h>
#include <string>
#include <vector>

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

	void Update(float dt) override;
	void Draw() override;

	void SetText(const std::string& t);
	std::string GetText() const;

	void SetFont(const std::string& f);
	std::string GetFont() const;

	void SetColor(const DirectX::XMFLOAT4& c);
	DirectX::XMFLOAT4 GetColor() const;

	Vec2 GetSize() const override;

	void SetFontSize(float s);
	float GetFontSize() const;
	void SetRightAligned(bool v);
	bool IsRightAligned() const;
	void SetMaxWidth(float w);
	float GetMaxWidth() const;

private:
	std::string text = "Text";
	std::string font;
	DirectX::XMFLOAT4 color{1.0f, 1.0f, 1.0f, 1.0f};
	// Persistent edit buffer for ImGui input to avoid single-character input issues
	std::vector<char> editBuffer;
	// Persistent edit buffer for the font field so ImGui keeps focus/state
	std::vector<char> fontEditBuffer;

	// Font size (in pixels) for rendering text
	float fontSize = 16.0f;
	bool rightAligned = false;
	float maxWidth = 0.0f;
};

} // namespace UI
