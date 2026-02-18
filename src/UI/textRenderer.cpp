#include "UI/textRenderer.h"
#include "core/filepathHolder.h"
#include "rendering/renderer.h"
#include "rendering/vertex.h"
#include "utilities/logger.h"
#include <WICTextureLoader.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <wrl.h>

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
	if (context == nullptr) return;
	Renderer* renderer = reinterpret_cast<Renderer*>(context);

	// Sort submissions by zIndex so higher zIndex draws later (on top)
	std::sort(submissions.begin(), submissions.end(), [](const TextSubmission& a, const TextSubmission& b) {
		return a.zIndex < b.zIndex;
	});

	// Render submissions in z order (low → high)
	for (const auto& sub : submissions) {
		std::string key = sub.font.empty() ? "default" : sub.font;

		// Ensure font is loaded
		if (!LoadFont(sub.font, renderer->GetDevice())) {
			Logger::Warn(Logger::LogMsg(std::string("Failed to load font: ") + key));
			continue;
		}

		auto& atlas = fonts.at(key);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		float scale = sub.fontSize / static_cast<float>(atlas.baseSize > 0 ? atlas.baseSize : 16);

		float cursorX = 0.0f;
		float startX = sub.position.x;
		float startY = sub.position.y;

		for (size_t i = 0; i < sub.text.size(); ++i) {
			unsigned char c = static_cast<unsigned char>(sub.text[i]);
			auto git = atlas.glyphs.find(static_cast<int>(c));
			if (git == atlas.glyphs.end()) {
				// advance by a default width if missing
				cursorX += 8.0f * scale;
				continue;
			}
			const Glyph& g = git->second;

			float gw = static_cast<float>(g.w) * scale;
			float gh = static_cast<float>(g.h) * scale;

			float x0 = startX + cursorX;
			float y0 = startY;
			float x1 = x0 + gw;
			float y1 = y0 + gh;

			// normalized UVs (sample texel centers to avoid missing edge rows with point sampling)
			float invW = 1.0f / static_cast<float>(atlas.atlasWidth);
			float invH = 1.0f / static_cast<float>(atlas.atlasHeight);
			float u0 = (static_cast<float>(g.x) + 0.5f) * invW;
			float v0 = (static_cast<float>(g.y) + 0.5f) * invH;
			float u1 = (static_cast<float>(g.x + g.w) - 0.5f) * invW;
			float v1 = (static_cast<float>(g.y + g.h) - 0.5f) * invH;

			// Append vertices (two triangles)
			uint32_t base = static_cast<uint32_t>(vertices.size());

			Vertex vTL{}; // top-left
			vTL.pos[0] = x0;
			vTL.pos[1] = y0;
			vTL.pos[2] = 0.0f;
			vTL.normal[0] = 0.0f;
			vTL.normal[1] = 0.0f;
			vTL.normal[2] = 0.0f;
			vTL.uv[0] = u0;
			vTL.uv[1] = 1.0f - v0;

			Vertex vTR{}; // top-right
			vTR.pos[0] = x1;
			vTR.pos[1] = y0;
			vTR.pos[2] = 0.0f;
			vTR.normal[0] = vTR.normal[1] = vTR.normal[2] = 0.0f;
			vTR.uv[0] = u1;
			vTR.uv[1] = 1.0f - v0;

			Vertex vBL{}; // bottom-left
			vBL.pos[0] = x0;
			vBL.pos[1] = y1;
			vBL.pos[2] = 0.0f;
			vBL.normal[0] = vBL.normal[1] = vBL.normal[2] = 0.0f;
			vBL.uv[0] = u0;
			vBL.uv[1] = 1.0f - v1;

			Vertex vBR{}; // bottom-right
			vBR.pos[0] = x1;
			vBR.pos[1] = y1;
			vBR.pos[2] = 0.0f;
			vBR.normal[0] = vBR.normal[1] = vBR.normal[2] = 0.0f;
			vBR.uv[0] = u1;
			vBR.uv[1] = 1.0f - v1;

			vertices.push_back(vTL);
			vertices.push_back(vTR);
			vertices.push_back(vBL);
			vertices.push_back(vBR);

			// indices: two triangles (TL, BL, BR) and (TL, BR, TR) but order depends on winding; using same as
			// elsewhere
			indices.push_back(base + 0);
			indices.push_back(base + 2);
			indices.push_back(base + 3);

			indices.push_back(base + 0);
			indices.push_back(base + 3);
			indices.push_back(base + 1);

			cursorX += static_cast<float>(g.xadvance) * scale;
		}

		if (!vertices.empty() && !indices.empty()) {
			if (!atlas.atlasSrv) {
				Logger::Log("TextRenderer: atlas SRV is null for font:", key);
			}
			// Use linear filtering for upscaled fonts for smoother result
			bool useLinear = scale > 1.0f + 1e-6f;
			renderer->DrawTextQuads(vertices, indices, atlas.atlasSrv.Get(), sub.color, useLinear);
		} else {
			Logger::Log("TextRenderer: no vertices generated for text:", sub.text);
		}
	}

	// Clear submissions so they don't accumulate across frames
	submissions.clear();
}

bool TextRenderer::LoadFont(const std::string& fontName, ID3D11Device* device) {
	namespace fs = std::filesystem;
	std::string key = fontName.empty() ? "default" : fontName;

	if (fonts.find(key) != fonts.end()) return true;

	fs::path assets = FilepathHolder::GetAssetsDirectory();
	fs::path fontDir = assets / "fonts" / (fontName.empty() ? "default" : fs::path(fontName));

	fs::path jsonPath = fontDir / "default_font.json";
	fs::path pngPath = fontDir / "default_font.png";

	if (!fs::exists(jsonPath) || !fs::exists(pngPath)) {
		Logger::Warn("TextRenderer: Font asset not found:", jsonPath.string());
		return false;
	}

	std::ifstream in(jsonPath);
	if (!in.is_open()) {
		Logger::Warn("TextRenderer: Failed to open font json:", jsonPath.string());
		return false;
	}

	nlohmann::json j;
	try {
		in >> j;
	} catch (const std::exception& e) {
		Logger::Warn("TextRenderer: Failed to parse font json:", e.what());
		return false;
	}

	FontAtlas atlas;
	atlas.baseSize = j.value("baseSize", 16);
	atlas.atlasWidth = j.value("atlasWidth", 0);
	atlas.atlasHeight = j.value("atlasHeight", 0);

	if (j.contains("glyphs")) {
		for (auto& it : j["glyphs"].items()) {
			int code = std::stoi(it.key());
			Glyph g{};
			auto v = it.value();
			g.x = v.value("x", 0);
			g.y = v.value("y", 0);
			g.w = v.value("w", 0);
			g.h = v.value("h", 0);
			g.xadvance = v.value("xadvance", 0);
			atlas.glyphs.emplace(code, g);
		}
	}

	// Load atlas PNG into a shader resource view
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	std::wstring wp = pngPath.wstring();
	HRESULT hr = DirectX::CreateWICTextureFromFile(device, wp.c_str(), nullptr, srv.GetAddressOf());
	if (FAILED(hr)) {
		Logger::Error("TextRenderer: Failed to create texture SRV for font:", pngPath.string());
		return false;
	}

	atlas.atlasSrv = srv;

	fonts.emplace(key, std::move(atlas));
	Logger::Log("TextRenderer: Loaded font atlas:", key);
	return true;
}

} // namespace UI
