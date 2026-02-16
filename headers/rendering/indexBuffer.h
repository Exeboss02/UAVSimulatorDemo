#pragma once

#include <cstdint>
#include <d3d11.h>
#include <format>
#include <wrl\client.h>

class IndexBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	size_t nrOfIndices = 0;

public:
	IndexBuffer() = default;
	~IndexBuffer() = default;
	IndexBuffer(const IndexBuffer& other) = default;
	IndexBuffer& operator=(const IndexBuffer& other) = default;
	IndexBuffer(IndexBuffer&& other) noexcept = default;
	IndexBuffer& operator=(IndexBuffer&& other) = default;

	void Init(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData);

	size_t GetNrOfIndices() const;
	ID3D11Buffer* GetBuffer() const;
};