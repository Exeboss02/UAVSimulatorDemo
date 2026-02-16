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

	void Init(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode);

	ID3D11SamplerState* GetSamplerState() const;
};