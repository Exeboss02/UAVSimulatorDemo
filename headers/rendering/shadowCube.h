#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <array>

class ShadowCube {
public:
	ShadowCube() = default;

	void Init(ID3D11Device* device, size_t res);

	ID3D11ShaderResourceView* GetSrv() const;
	ID3D11DepthStencilView* GetDsv(size_t index) const;

private:
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	std::array<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>, 6> depthStencilViews;
};

