#pragma once

#include <d3d11.h>
#include <format>
#include <iostream>
#include <wrl/client.h>

class RenderTarget {
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

public:
	RenderTarget() = default;
	~RenderTarget() = default;

	void Init(ID3D11Device* device, IDXGISwapChain* swapChain);

	ID3D11RenderTargetView* GetRenderTargetView();
	ID3D11ShaderResourceView* GetShaderResourceView() const;
};