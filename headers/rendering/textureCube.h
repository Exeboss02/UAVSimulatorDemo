#pragma once

#include <d3d11.h>
#include <DDSTextureLoader.h>

#include "rendering/renderTarget.h"

class TextureCube {
private:
	Microsoft::WRL::ComPtr<ID3D11Resource> texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	void CreateTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string filepath);

public:
	TextureCube() = default;
	~TextureCube() = default;

	void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string ddsPath);

	ID3D11ShaderResourceView* GetShaderResourceView() const;
};