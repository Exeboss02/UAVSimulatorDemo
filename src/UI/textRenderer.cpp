#include "UI/textRenderer.h"
#include "core/filepathHolder.h"
#include "rendering/renderer.h"
#include "rendering/vertex.h"
#include "utilities/logger.h"

#include "FW1FontWrapper.h"

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
		std::wstring wfont(sub.font.empty() ? L"Arial" : std::wstring(sub.font.begin(), sub.font.end()));

		if (wtext.empty()) {
			Logger::Warn("TextRenderer::Render: wtext is empty!");
			continue;
		}
		if (wfont.empty()) {
			Logger::Warn("TextRenderer::Render: wfont is empty! Using Arial.");
			wfont = L"Arial";
		}
		float safeFontSize = sub.fontSize > 0 ? sub.fontSize : 16.0f;
		float safeX = std::max(0.0f, sub.position.x);
		float safeY = std::max(0.0f, sub.position.y);
		UINT color = ((UINT) (sub.color.w * 255) << 24) | ((UINT) (sub.color.x * 255) << 16) |
					 ((UINT) (sub.color.y * 255) << 8) | ((UINT) (sub.color.z * 255));

		Logger::Log("TextRenderer::Render: Drawing text:", sub.text, "font:", std::string(wfont.begin(), wfont.end()),
					"size:", safeFontSize, "x:", safeX, "y:", safeY);
		fw1FontWrapper->DrawString(deviceContext, wtext.c_str(), wfont.c_str(), safeFontSize, safeX, safeY, color,
								   FW1_RESTORESTATE);
		Logger::Log("TextRenderer::Render: DrawString completed.");
	}
	submissions.clear();
}

bool TextRenderer::InitializeFW1(ID3D11Device* device) {
	if (fw1Initialized) return true;
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &fw1Factory);
	if (FAILED(hr) || !fw1Factory) {
		Logger::Error("Failed to create FW1Factory. HRESULT=", hr);
		return false;
	}
	hr = fw1Factory->CreateFontWrapper(device, L"assets/fonts/lucon.ttf", &fw1FontWrapper);
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
