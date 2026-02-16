#pragma once

#include "utilities/logger.h"
#include <d3d11.h>
#include <format>
#include <iostream>
#include <wrl/client.h>

class RasterizerState {
public:
	RasterizerState() = default;
	~RasterizerState() = default;

	void Init(ID3D11Device* device, const D3D11_RASTERIZER_DESC* desc);

	ID3D11RasterizerState* GetRasterizerState();

private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	;
};