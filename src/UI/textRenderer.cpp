#include "UI/textRenderer.h"
#include "FW1FontWrapper.h"
#include "utilities/logger.h"
#include <algorithm>

namespace UI {

TextRenderer& TextRenderer::GetInstance() {
	static TextRenderer inst;
	return inst;
}

void TextRenderer::SubmitText(const std::string& text, Vec2 position, float fontSize, DirectX::XMFLOAT4 color,
							  const std::string& font, int zIndex) {
	TextSubmission s;
	s.text = text;
	s.position = position;
	s.fontSize = fontSize;
	s.color = color;
	s.font = font;
	s.zIndex = zIndex;
	submissions.push_back(std::move(s));
}

void TextRenderer::ClearSubmissions() { submissions.clear(); }

void TextRenderer::Render(void* context) {
	if (!fw1Initialized || fw1FontWrapper == nullptr) {
		Logger::Warn("FW1FontWrapper not initialized!");
		return;
	}
	if (context == nullptr) {
		Logger::Error("TextRenderer::Render called with null context!");
		return;
	}
	ID3D11DeviceContext* deviceContext = reinterpret_cast<ID3D11DeviceContext*>(context);
	if (!deviceContext) {
		Logger::Error("TextRenderer::Render: deviceContext is null!");
		return;
	}

	std::sort(submissions.begin(), submissions.end(),
			  [](const TextSubmission& a, const TextSubmission& b) { return a.zIndex < b.zIndex; });

	for (const auto& sub : submissions) {
		std::wstring wtext(sub.text.begin(), sub.text.end());
		std::wstring wfont(sub.font.empty() ? L"Lucida Console" : std::wstring(sub.font.begin(), sub.font.end()));

		if (wtext.empty()) {
			Logger::Warn("TextRenderer::Render: wtext is empty!");
			continue;
		}
		if (wfont.empty()) {
			Logger::Warn("TextRenderer::Render: wfont is empty! Using assets/fonts/lucon.ttf.");
			wfont = L"Lucida Console";
		}

		float safeFontSize = sub.fontSize > 0 ? sub.fontSize : 16.0f;
		float safeX = std::max(0.0f, sub.position.x);
		float safeY = std::max(0.0f, sub.position.y);

		UINT color = ((UINT) (sub.color.w * 255) << 24) | ((UINT) (sub.color.z * 255) << 16) |
					 ((UINT) (sub.color.y * 255) << 8) | ((UINT) (sub.color.x * 255));

		this->MeasureString(sub.text, sub.fontSize, sub.font);
		Logger::Log("Position: (", safeX, ":", safeY, ")");

		fw1FontWrapper->DrawString(deviceContext, wtext.c_str(), wfont.c_str(), safeFontSize, safeX, safeY, color,
								   FW1_RESTORESTATE);
	}
	submissions.clear();
}

float TextRenderer::MeasureString(const std::string& text, float fontSize, const std::string& font) {
	if (!fw1Initialized || fw1FontWrapper == nullptr) return 0.0f;
	if (text.empty()) return 0.0f;

	std::wstring wtext(text.begin(), text.end());
	std::wstring wfont(font.empty() ? L"Lucida Console" : std::wstring(font.begin(), font.end()));

	FW1_RECTF layout{};
	layout.Left = 0;
	layout.Top = 0;
	layout.Right = 1000.;
	layout.Bottom = 1000.;
	FW1_RECTF rect = fw1FontWrapper->MeasureString(wtext.c_str(), wfont.c_str(), fontSize, &layout, 0);

	Logger::Log("Right: ", rect.Right, ", Left: ", rect.Left, ", Font: ", font, ", FontSize: ", fontSize, ", Text: ", text);

	return rect.Right - rect.Left;
}

bool TextRenderer::InitializeFW1(ID3D11Device* device) {
	if (fw1Initialized) return true;
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &fw1Factory);
	if (FAILED(hr) || !fw1Factory) {
		Logger::Error("Failed to create FW1Factory. HRESULT=", hr);
		return false;
	}
	hr = fw1Factory->CreateFontWrapper(device, L"Lucida Console", &fw1FontWrapper);
	if (FAILED(hr) || !fw1FontWrapper) {
		Logger::Error("Failed to create FW1FontWrapper. HRESULT=", hr);
		fw1Factory->Release();
		fw1Factory = nullptr;
		return false;
	}
	fw1Initialized = true;
	Logger::Log("FW1FontWrapper initialized");
	return true;
}

} // namespace UI
