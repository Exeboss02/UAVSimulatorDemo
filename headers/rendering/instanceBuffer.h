#pragma once

#include <d3d11.h>
#include <format>
#include <iostream>
#include <wrl/client.h>
#include "rendering/inputLayout.h"
#include "rendering/renderMap.h"

class InstanceBuffer {
public:
	void Init(ID3D11Device* device, UINT sizeOfInstance, UINT nrOfInstances, void* instances);

	void Update(ID3D11DeviceContext* context, UINT sizeOfInstance, UINT nrOfInstances, void* data);

	UINT GetNrOfInstances() const;
	UINT GetInstanceSize() const;
	ID3D11Buffer* GetBuffer() const;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer = nullptr;
	UINT nrOfInstances = 0;
	UINT intanceSize = 0;
};