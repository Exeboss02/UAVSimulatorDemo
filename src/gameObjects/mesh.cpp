#include "gameObjects/mesh.h"

Mesh::Mesh(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes,
		   DirectX::BoundingBox boundingBox)
	: vertexbuffer(std::move(vertexbuffer)), indexbuffer(std::move(indexbuffer)), subMeshes(std::move(submeshes)),
	  boundingBox(boundingBox) {}

Mesh::~Mesh() {}

void Mesh::Init(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes) {
	this->Init(std::move(vertexbuffer), std::move(indexbuffer), std::move(submeshes), DirectX::BoundingBox{});
}

std::string& Mesh::GetIdentifier() { return this->identifier; }

std::vector<SubMesh>& Mesh::GetSubMeshes() { return this->subMeshes; }

VertexBuffer& Mesh::GetVertexBuffer() { return this->vertexbuffer; }

IndexBuffer& Mesh::GetIndexBuffer() { return this->indexbuffer; }

void Mesh::SetIdentifier(std::string name) { this->identifier = name; }

const DirectX::BoundingBox& Mesh::GetBoundingBox() const { return this->boundingBox; }

SubMesh::SubMesh(size_t startIndex, size_t nrOfIndices) : startIndex(startIndex), nrOfIndices(nrOfIndices) {}
SubMesh::~SubMesh() {}

size_t SubMesh::GetStartIndex() const { return this->startIndex; }

size_t SubMesh::GetNrOfIndices() const { return this->nrOfIndices; }

void Mesh::Init(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes,
				DirectX::BoundingBox boundingBox) {
	this->vertexbuffer = std::move(vertexbuffer);
	this->indexbuffer = std::move(indexbuffer);
	this->subMeshes = std::move(submeshes);
	this->boundingBox = boundingBox;
}
