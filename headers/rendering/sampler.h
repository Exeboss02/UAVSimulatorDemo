#pragma once

#include <array>
#include <optional> // Needs C++ 17 or higher
#include <wrl/client.h>

#include <d3d11.h>

#include <format>

class Sampler {
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler = nullptr;

public:
	Sampler() = default;
	~Sampler() = default;
	Sampler(const Sampler& other) = delete;
	Sampler& operator=(const Sampler& other) = delete;
	Sampler(Sampler&& other) = delete;
	Sampler& operator=(Sampler&& other) = delete;

	void Init(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode,
			  D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			  D3D11_COMPARISON_FUNC compFunc = D3D11_COMPARISON_NEVER, std::array<float, 4> borderColor = {0, 0, 0, 1});

	ID3D11SamplerState* GetSamplerState() const;
};