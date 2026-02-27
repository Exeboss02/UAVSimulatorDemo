#pragma once

#include "rendering/indexBuffer.h"
#include "rendering/texture.h"
#include "rendering/vertex.h"
#include "rendering/vertexBuffer.h"
#include <DirectXCollision.h>
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
	Mesh(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes,
		 DirectX::BoundingBox boundingBox);
	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(Mesh&&) noexcept = default;
	~Mesh();

	void Init(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes,
			  DirectX::BoundingBox boundingBox);

	std::vector<SubMesh>& GetSubMeshes();
	VertexBuffer& GetVertexBuffer();
	IndexBuffer& GetIndexBuffer();
	std::string& GetIdentifier();
	const DirectX::BoundingBox& GetBoundingBox() const;

	void SetIdentifier(std::string identifier);

	size_t GetMeshIndex();

private:
	DirectX::BoundingBox boundingBox;
	std::string identifier;
	size_t meshIndex; // Used as a faster identifier
	std::vector<SubMesh> subMeshes;
	VertexBuffer vertexbuffer;
	IndexBuffer indexbuffer;
};
