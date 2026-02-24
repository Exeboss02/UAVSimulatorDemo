#pragma once

#include "UI/widget.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <wrl.h>

// FW1FontWrapper forward declarations
struct IFW1Factory;
struct IFW1FontWrapper;

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace UI {

struct TextSubmission {
	std::string text;
	Vec2 position;
	float fontSize;
	DirectX::XMFLOAT4 color;
	std::string font;
	int zIndex = 0;
};

class TextRenderer {
public:
	static TextRenderer& GetInstance();

	void SubmitText(const std::string& text, Vec2 position, float fontSize, DirectX::XMFLOAT4 color,
					const std::string& font = "", int zIndex = 0);

	void ClearSubmissions();
	void Render(void* context);

	// Initialize FW1FontWrapper (call once, after device is created)
	bool InitializeFW1(ID3D11Device* device);

private:
	std::vector<TextSubmission> submissions;

	IFW1Factory* fw1Factory = nullptr;
	IFW1FontWrapper* fw1FontWrapper = nullptr;
	bool fw1Initialized = false;
};

} // namespace UI
