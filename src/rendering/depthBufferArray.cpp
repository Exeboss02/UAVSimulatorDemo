#include "rendering/depthBufferArray.h"
#include "utilities/logger.h"

void DepthBufferArray::Init(ID3D11Device* device, UINT resolution, UINT maxViews) {

	if (this->texture.Get()) this->texture.Reset();
	if (this->shaderResourceView.Get()) this->shaderResourceView.Reset();
	for (auto& depthStencilView : this->depthStencilViews) {
		if (depthStencilView.Get()) depthStencilView.Reset();
	}
	this->depthStencilViews.resize(maxViews);

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = resolution;
	textureDesc.Height = resolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = maxViews;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, this->texture.GetAddressOf());
	if (FAILED(hr)) {
		throw std::exception(
			std::format("Failed to create Depth Stencil Texture, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr))
				.c_str());
	}



	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	// Depth view uses 32-bit float depth format
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.MipSlice = 0;

	for (size_t i = 0; i < maxViews; i++) {
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateDepthStencilView(texture.Get(), &dsvDesc, this->depthStencilViews[i].GetAddressOf());

		if (FAILED(hr)) {
			std::string error = std::format("Failed to create shadow depth stencil view, HRESULT: 0x{:08X}",
											static_cast<unsigned long>(hr));
			Logger::Error(error);
			throw std::exception(error.c_str());
		}
	}

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    // SRV reads the depth values as a single R component float
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = maxViews;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, this->shaderResourceView.GetAddressOf());
	if (FAILED(hr)) {
		std::string error = std::format("Failed to create shadow shader resource view, HRESULT: 0x{:08X}",
										static_cast<unsigned long>(hr));
		Logger::Error(error);
		throw std::exception(error.c_str());
	}
}

ID3D11DepthStencilView* DepthBufferArray::GetDepthStencilView(UINT arrayIndex) const {
	if (arrayIndex >= this->depthStencilViews.size()) {
		std::string error = std::format("Array index {} is out of bounds for depth stencil views array of size {}",
										arrayIndex, this->depthStencilViews.size());
		Logger::Error(error);
		throw std::runtime_error(error);
	}
	return this->depthStencilViews[arrayIndex].Get();
}

ID3D11ShaderResourceView* DepthBufferArray::GetShaderResourceView() const { return this->shaderResourceView.Get(); }
