#pragma once

#include <d3d11.h>
#include <format>
#include <iostream>
#include <wrl/client.h>

class VertexBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer = nullptr;
	UINT nrOfVertices = 0;
	UINT vertexSize = 0;

	bool initialized = false;

public:
	VertexBuffer() = default;
	~VertexBuffer() = default;
	VertexBuffer(const VertexBuffer& other) = default;
	VertexBuffer& operator=(const VertexBuffer& other) = default;
	VertexBuffer(VertexBuffer&& other) noexcept = default;
	VertexBuffer& operator=(VertexBuffer&& other) = default;

	/// <summary>
	/// Initializes the vertex buffer with data
	/// </summary>
	/// <param name="device"></param>
	/// <param name="sizeOfVertex">Size of one vertex in bytes</param>
	/// <param name="nrOfVerticesInBuffer">Number of vertices</param>
	/// <param name="vertexData">An array of all vertices with correct layout e.g. position, normals, uv</param>
	void Init(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData, bool needToUpdate = false);

	/// <summary>
	/// You have to really be sure of what you're doing to use this or it will lead to undefined behaviour
	/// </summary>
	void Update(ID3D11DeviceContext* context, void* vertexData);

	bool IsInitialized();

	UINT GetNrOfVertices() const;
	UINT GetVertexSize() const;
	ID3D11Buffer* GetBuffer() const;
};