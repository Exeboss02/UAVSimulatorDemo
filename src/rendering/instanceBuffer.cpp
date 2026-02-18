#include "rendering/instanceBuffer.h"
#include <format>
#include "imgui.h"

void InstanceBuffer::Init(ID3D11Device* device, UINT sizeOfInstance, UINT nrOfInstances, void* instances) {
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeOfInstance * nrOfInstances;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = instances;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, this->buffer.GetAddressOf());
	if (FAILED(hr)) {
		Logger::Log(std::format("Failed to create instance buffer, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)));
		throw std::exception("Fatal error in isntance buffer");
	}

	this->nrOfInstances = nrOfInstances;
	this->intanceSize = sizeOfInstance;
}

void InstanceBuffer::Update(ID3D11DeviceContext* context, UINT sizeOfInstance, UINT nrOfInstances, void* data) {
	if (this->nrOfInstances * this->intanceSize < sizeOfInstance * nrOfInstances) {
		std::string error = "Trying to write more data than buffer size.";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Turn off GPU access
	context->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy(mappedResource.pData, data, this->nrOfInstances * this->intanceSize);

	context->Unmap(this->buffer.Get(), 0);
	// Turn on GPU access
}

UINT InstanceBuffer::GetNrOfInstances() const { return this->nrOfInstances; }

UINT InstanceBuffer::GetInstanceSize() const { return this->intanceSize; }

ID3D11Buffer* InstanceBuffer::GetBuffer() const { return this->buffer.Get(); }
