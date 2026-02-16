#pragma once

#include <d3d11.h>
#include <format>
#include <iostream>
#include <string>
#include <wrl/client.h>

class ConstantBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	size_t bufferSize = 0;

public:
	ConstantBuffer() = default;
	~ConstantBuffer() = default;
	ConstantBuffer(const ConstantBuffer& other) = delete;
	ConstantBuffer& operator=(const ConstantBuffer& other) = delete;
	ConstantBuffer(ConstantBuffer&& other) noexcept = default;
	ConstantBuffer& operator=(ConstantBuffer&& other) noexcept = default;

	/// <summary>
	/// Intialize the constant buffer with data
	/// </summary>
	/// <param name="device"></param>
	/// <param name="byteSize">The size of the data in bytes</param>
	/// <param name="initialData">Data to initialize with, usually a struct</param>
	/// <param name="usage">GPU/CPU write/read access</param>
	/// <param name="cpuAccess">CPU write/read access</param>
	void Init(ID3D11Device* device, size_t byteSize, void* initialData, D3D11_USAGE usage, UINT cpuAccess);

	size_t GetSize() const;
	ID3D11Buffer* GetBuffer() const;

	void UpdateBuffer(ID3D11DeviceContext* context, void* data);
};