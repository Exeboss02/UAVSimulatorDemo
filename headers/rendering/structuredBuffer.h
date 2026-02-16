#pragma once

#include "utilities/logger.h"
#include <d3d11.h>
#include <format>
#include <wrl/client.h>

template <class T>
class StructuredBuffer {
public:
	StructuredBuffer() = default;
	~StructuredBuffer() = default;
	StructuredBuffer(const StructuredBuffer& other) = default;
	StructuredBuffer& operator=(const StructuredBuffer& other) = default;
	StructuredBuffer(StructuredBuffer&& other) = default;
	// StructuredBuffer operator=(StructuredBuffer&& other) = default;

	UINT GetElementSize() const;
	size_t GetNrOfElements() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11UnorderedAccessView* GetUAV() const;

	void Init(ID3D11Device* device, size_t nrOfElementsInBuffer, std::vector<T> initalData, bool dynamic = true,
			  bool hasSRV = true, bool hasUAV = false) {

		initalData.reserve(nrOfElementsInBuffer);
		this->_Init(device, sizeof(T), nrOfElementsInBuffer, initalData.data(), sizeof(T) * initalData.size(), dynamic,
					hasSRV, hasUAV);
	};

	void UpdateBuffer(ID3D11DeviceContext* context, std::vector<T> data) {
		size_t size = data.size();
		if (data.size() > this->nrOfElements) {
			std::string error = "Trying to write more data than buffer size, disregarding elements";
			Logger::Error(error);
			size = this->nrOfElements;
		}

		this->_UpdateBuffer(context, data.data(), sizeof(T) * size);
	};

private:
	void _Init(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, void* bufferData = nullptr,
			   size_t initialDataSize = 0, bool dynamic = true, bool hasSRV = true, bool hasUAV = false);

	void _UpdateBuffer(ID3D11DeviceContext* context, void* data, size_t bytes);
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv = nullptr;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav = nullptr;
	UINT elementSize = 0;
	size_t nrOfElements = 0;
};

template <class T>
void StructuredBuffer<T>::_Init(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, void* bufferData,
								size_t initialDataBytes, bool dynamic, bool hasSRV, bool hasUAV) {
	this->elementSize = sizeOfElement;
	this->nrOfElements = nrOfElementsInBuffer;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = this->elementSize * this->nrOfElements;
	bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;	  // SRV
	bufferDesc.BindFlags |= hasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0; // UAV
	bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = this->elementSize;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = bufferData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, this->buffer.GetAddressOf());
	if (FAILED(hr)) {
		throw std::exception(
			std::format("Failed to create structured buffer, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr))
				.c_str());
	}

	if (hasSRV) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;				// This is important for structured buffer
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; // Is a buffer
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = this->nrOfElements;
		// srvDesc.Buffer.ElementWidth = this->elementSize;

		HRESULT hr2 = device->CreateShaderResourceView(this->buffer.Get(), &srvDesc, this->srv.GetAddressOf());
		if (FAILED(hr2)) {
			throw std::exception(std::format("Failed to create structured buffer srv, HRESULT: 0x{:08X}",
											 static_cast<unsigned long>(hr2))
									 .c_str());
		}
	}

	if (hasUAV) {
		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;				// This is important for structured buffer
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // Is a buffer
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = this->nrOfElements;
		uavDesc.Buffer.Flags = 0;

		HRESULT hr2 = device->CreateUnorderedAccessView(this->buffer.Get(), &uavDesc, this->uav.GetAddressOf());
		if (FAILED(hr2)) {
			throw std::exception(std::format("Failed to create structured buffer uav, HRESULT: 0x{:08X}",
											 static_cast<unsigned long>(hr2))
									 .c_str());
		}
	}
}
template <class T>
void StructuredBuffer<T>::_UpdateBuffer(ID3D11DeviceContext* context, void* data, size_t bytes) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Turn off GPU access
	context->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy(mappedResource.pData, data, bytes);

	context->Unmap(this->buffer.Get(), 0);
	// Turn on GPU access
}
template <class T>
UINT StructuredBuffer<T>::GetElementSize() const {
	return this->elementSize;
}

template <class T>
size_t StructuredBuffer<T>::GetNrOfElements() const {
	return this->nrOfElements;
}
template <class T>
ID3D11ShaderResourceView* StructuredBuffer<T>::GetSRV() const {
	return this->srv.Get();
}

template <class T>
ID3D11UnorderedAccessView* StructuredBuffer<T>::GetUAV() const {
	return this->uav.Get();
}