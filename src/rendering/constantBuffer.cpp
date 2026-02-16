#include "rendering/constantBuffer.h"

#include <cassert>

void ConstantBuffer::Init(ID3D11Device* device, size_t byteSize, void* initialData, D3D11_USAGE usage, UINT cpuAccess) {
	assert(byteSize % 16 == 0 && "ConstantBuffer byte size must be 16 byte aligned");
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = byteSize;
	bufferDesc.Usage = usage;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = cpuAccess;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initialData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, this->buffer.GetAddressOf());
	if (FAILED(hr)) {
		throw std::exception(
			std::format("Failed to create constant buffer, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}

	this->bufferSize = byteSize;
}

size_t ConstantBuffer::GetSize() const { return this->bufferSize; }

ID3D11Buffer* ConstantBuffer::GetBuffer() const { return this->buffer.Get(); }

void ConstantBuffer::UpdateBuffer(ID3D11DeviceContext* context, void* data) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Turn off GPU access
	HRESULT hr = context->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) {
		throw std::exception(
			std::format("Failed to map resource, error: HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}

	memcpy(mappedResource.pData, data, this->bufferSize);

	context->Unmap(this->buffer.Get(), 0);
	// Turn on GPU access
}