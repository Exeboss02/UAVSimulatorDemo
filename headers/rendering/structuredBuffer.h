#pragma once

#include "utilities/logger.h"
#include <d3d11.h>
#include <format>
#include <wrl/client.h>

class StructuredBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv = nullptr;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav = nullptr;
	UINT elementSize = 0;
	size_t nrOfElements = 0;

public:
	StructuredBuffer() = default;
	~StructuredBuffer() = default;
	StructuredBuffer(const StructuredBuffer& other) = default;
	StructuredBuffer& operator=(const StructuredBuffer& other) = default;
	StructuredBuffer(StructuredBuffer&& other) = default;
	// StructuredBuffer operator=(StructuredBuffer&& other) = default;

	void Init(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, void* bufferData = nullptr,
			  bool dynamic = true, bool hasSRV = true, bool hasUAV = false);

	void UpdateBuffer(ID3D11DeviceContext* context, void* data);

	UINT GetElementSize() const;
	size_t GetNrOfElements() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11UnorderedAccessView* GetUAV() const;
};