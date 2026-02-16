#pragma once

#include <d3d11.h>
#include <format>
#include <vector>
#include <wrl/client.h>

class DepthBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilViews;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

public:
	DepthBuffer() = default;
	~DepthBuffer() = default;

	void Init(ID3D11Device* device, UINT width, UINT height);

	ID3D11DepthStencilView* GetDepthStencilView(UINT arrayIndex) const;
	ID3D11DepthStencilView* GetDepthStencilView() const;
	ID3D11ShaderResourceView* GetShaderResourceView() const;
	ID3D11DepthStencilState* GetDepthStencilState() const;
};