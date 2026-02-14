#pragma once

#include "rendering/indexBuffer.h"
#include "rendering/texture.h"
#include "rendering/vertex.h"
#include "rendering/vertexBuffer.h"
#include <array>
#include <string>
#include <vector>

class SubMesh {
public:
	SubMesh(size_t startIndex, size_t nrOfIndices);
	SubMesh(SubMesh&&) noexcept = default;
	SubMesh(SubMesh&) = default;
	SubMesh& operator=(SubMesh&&) noexcept = default;
	SubMesh& operator=(SubMesh&) = default;
	~SubMesh();

	size_t GetStartIndex() const;
	size_t GetNrOfIndices() const;

private:
	size_t startIndex;
	size_t nrOfIndices;
};

class Mesh {
public:
	Mesh() = default;
	Mesh(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes);
	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(Mesh&&) noexcept = default;
	~Mesh();

	void Init(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes);

	std::vector<SubMesh>& GetSubMeshes();
	VertexBuffer& GetVertexBuffer();
	IndexBuffer& GetIndexBuffer();

	std::string& GetIdentifier();
	void SetIdentifier(std::string identifier);

private:
	std::string identifier;
	std::vector<SubMesh> subMeshes;
	VertexBuffer vertexbuffer;
	IndexBuffer indexbuffer;
};
