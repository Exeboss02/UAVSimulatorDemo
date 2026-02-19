#pragma once

#include <d3d11.h>
#include <format>
#include <vector>
#include <wrl/client.h>

class DepthBufferArray {
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> depthStencilViews;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

public:
	DepthBufferArray() = default;
	~DepthBufferArray() = default;

	void Init(ID3D11Device* device, UINT width, UINT height, UINT maxViews);

	ID3D11DepthStencilView* GetDepthStencilView(UINT arrayIndex) const;
	ID3D11ShaderResourceView* GetShaderResourceView() const;
	ID3D11DepthStencilState* GetDepthStencilState() const;
};