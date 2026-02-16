#include "rendering/sampler.h"

void Sampler::Init(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode, D3D11_FILTER filter,
				   D3D11_COMPARISON_FUNC compFunc, std::array<float, 4> borderColor) {
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = filter; // Uses linear interpolation // No
	samplerDesc.AddressU = adressMode; // Uses wrap for all sides
	samplerDesc.AddressV = adressMode;
	samplerDesc.AddressW = adressMode; // Is this even used for a 2D texture?
	samplerDesc.MipLODBias = 0; // Only have on miplevel, so doesn't really matter, but it's an offset from calculated mipmaplevel
	samplerDesc.MaxAnisotropy = 1; // Unused
	samplerDesc.ComparisonFunc = compFunc; // Isn't really relevant for this usecase // No
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	samplerDesc.BorderColor[0] = borderColor[0];
	samplerDesc.BorderColor[1] = borderColor[1];
	samplerDesc.BorderColor[2] = borderColor[2];
	samplerDesc.BorderColor[3] = borderColor [3];
	

	HRESULT hr = device->CreateSamplerState(&samplerDesc, &sampler);

	if (FAILED(hr)) {
		throw std::exception(std::format("Failed to create the sampler state, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}
}

ID3D11SamplerState* Sampler::GetSamplerState() const
{
	return this->sampler.Get();
}
