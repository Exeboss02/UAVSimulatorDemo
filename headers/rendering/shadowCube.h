#pragma once
#include <array>
#include <d3d11.h>
#include <vector>
#include <wrl.h>

class ShadowCube {
public:
	ShadowCube() = default;

	void Init(ID3D11Device* device, size_t resolution, size_t maxShadowCubes);

	ID3D11ShaderResourceView* GetSrv() const;
	ID3D11DepthStencilView* GetDsv(size_t cubeIndex, size_t faceIndex) const;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	std::vector<std::array<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>, 6>> depthStencilViews;
};
