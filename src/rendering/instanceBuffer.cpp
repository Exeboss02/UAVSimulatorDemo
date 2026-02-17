#include "rendering/instanceBuffer.h"
#include <format>
#include "imgui.h"

void InstanceBuffer::Init(ID3D11Device* device, UINT sizeOfInstance, UINT nrOfInstances, void* instances) {
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeOfInstance * nrOfInstances;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
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

UINT InstanceBuffer::GetNrOfInstances() const { return this->nrOfInstances; }

UINT InstanceBuffer::GetInstanceSize() const { return this->intanceSize; }

ID3D11Buffer* InstanceBuffer::GetBuffer() const { return this->buffer.Get(); }
