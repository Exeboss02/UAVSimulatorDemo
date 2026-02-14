#include "rendering/textureCube.h"
#include "utilities/logger.h"

void TextureCube::CreateTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string filepath) 
{
	const std::wstring wstr = std::wstring(filepath.begin(), filepath.end());
	HRESULT hr = DirectX::CreateDDSTextureFromFileEx(device, deviceContext, wstr.c_str(), 0,
										D3D11_USAGE_DEFAULT,
										D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, DirectX::DX11::DDS_LOADER_DEFAULT,
										this->texture.GetAddressOf(), this->shaderResourceView.GetAddressOf());

	if (FAILED(hr)) {
		std::string output = std::format("Failed to create Texture Cube, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr));
		Logger::Error(output);
		throw std::exception(output.c_str());
	}
}

void TextureCube::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string ddsPath) {
	CreateTextures(device, deviceContext, ddsPath);
}

ID3D11ShaderResourceView* TextureCube::GetShaderResourceView() const {
	return this->shaderResourceView.Get();
}
