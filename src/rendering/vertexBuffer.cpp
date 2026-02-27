#include "rendering/vertexBuffer.h"
#include "utilities/logger.h"

void VertexBuffer::Init(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData, bool needToUpdate)
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeOfVertex * nrOfVerticesInBuffer;
	bufferDesc.Usage = needToUpdate ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = needToUpdate ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, this->buffer.GetAddressOf());
	if (FAILED(hr)) {
		throw std::exception(std::format("Failed to create vertex buffer, HRESULT: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}

	this->vertexSize = sizeOfVertex;
	this->nrOfVertices = nrOfVerticesInBuffer;

	this->initialized = true;
}

void VertexBuffer::Update(ID3D11DeviceContext* context, void* vertexData) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (!this->IsInitialized()) {
		Logger::Error("Unitiliazed buffer");
		throw std::runtime_error("Fatal error in Vertex Buffer");
	}

	// Turn off GPU access
	HRESULT hr = context->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) {
		Logger::Error(std::format("Failed to map, error: 0x{:08X}", static_cast<unsigned long>(hr)).c_str());
	}

	memcpy(mappedResource.pData, vertexData, this->nrOfVertices * this->vertexSize);

	context->Unmap(this->buffer.Get(), 0);
	// Turn on GPU access
}

bool VertexBuffer::IsInitialized() { return this->initialized; }

UINT VertexBuffer::GetNrOfVertices() const
{
	return this->nrOfVertices;
}

UINT VertexBuffer::GetVertexSize() const
{
	return this->vertexSize;
}

ID3D11Buffer* VertexBuffer::GetBuffer() const
{
	return this->buffer.Get();
}
