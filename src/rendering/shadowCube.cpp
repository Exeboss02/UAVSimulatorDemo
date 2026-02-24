#include "rendering/shadowCube.h"
#include "utilities/logger.h"
#include <format>
#include <stdexcept>
#include <string>

void ShadowCube::Init(ID3D11Device* device, size_t resolution, size_t maxShadowCubes) {
	this->shaderResourceView.Reset();
	for (auto& depthstencilCube : this->depthStencilViews) {
		for (auto& depthstencil : depthstencilCube) {
			depthstencil.Reset();
		}
	}
	this->depthStencilViews.resize(maxShadowCubes);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = resolution;
	desc.Height = resolution;
	desc.MipLevels = 1;
	desc.ArraySize = 6 * maxShadowCubes;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
	if (FAILED(hr)) {
		std::string error =
			std::format("Failed to create shadow cube texture, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr));
		Logger::Error(error);
		throw std::exception(error.c_str());
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.TextureCubeArray.First2DArrayFace = 0;
	srvDesc.TextureCubeArray.NumCubes = maxShadowCubes;
	srvDesc.TextureCube.MipLevels = 1;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, this->shaderResourceView.GetAddressOf());
	if (FAILED(hr)) {
		std::string error = std::format("Failed to create shadow shader resource view, HRESULT: 0x{:08X}",
										static_cast<unsigned long>(hr));
		Logger::Error(error);
		throw std::exception(error.c_str());
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.MipSlice = 0;
	for (size_t cube = 0; cube < this->depthStencilViews.size(); cube++) {
		for (size_t i = 0; i < 6; i++) {
			dsvDesc.Texture2DArray.FirstArraySlice = cube * 6 + i;
			hr = device->CreateDepthStencilView(texture.Get(), &dsvDesc,
												this->depthStencilViews[cube][i].GetAddressOf());

			if (FAILED(hr)) {
				std::string error = std::format("Failed to create shadow depth stencil view, HRESULT: 0x{:08X}",
												static_cast<unsigned long>(hr));
				Logger::Error(error);
				throw std::exception(error.c_str());
			}
		}
	}
}

ID3D11ShaderResourceView* ShadowCube::GetSrv() const { return this->shaderResourceView.Get(); }

ID3D11DepthStencilView* ShadowCube::GetDsv(size_t cubeIndex, size_t faceIndex) const {
	if (cubeIndex >= this->depthStencilViews.size()) {
		std::string error = "Trying to access depthstencilview out of bounds";
		Logger::Error(error);
		throw std::runtime_error(error);
	}
	if (faceIndex >= 6) {
		std::string error = "Trying to access depthstencilview out of bounds";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	return this->depthStencilViews[cubeIndex][faceIndex].Get();
}
