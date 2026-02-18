#pragma once

#include "UI/widget.h"
#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>

struct ID3D11ShaderResourceView;
struct ID3D11Device;

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

	// Load a font atlas (JSON + PNG) into memory. Returns true on success.
	bool LoadFont(const std::string& fontName, ID3D11Device* device);

private:
	std::vector<TextSubmission> submissions;

	struct Glyph {
		int x;
		int y;
		int w;
		int h;
		int xadvance;
	};

	struct FontAtlas {
		int baseSize = 0;
		int atlasWidth = 0;
		int atlasHeight = 0;
		std::unordered_map<int, Glyph> glyphs;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> atlasSrv;
	};

	std::unordered_map<std::string, FontAtlas> fonts;
};

} // namespace UI
