#include "gameObjects/mesh.h"

Mesh::Mesh(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes,
		   DirectX::BoundingBox boundingBox)
	: vertexbuffer(std::move(vertexbuffer)), indexbuffer(std::move(indexbuffer)), subMeshes(std::move(submeshes)),
	  boundingBox(boundingBox), meshIndex(0) {}

Mesh::~Mesh() {}

std::string& Mesh::GetIdentifier() { return this->identifier; }

const DirectX::BoundingBox& Mesh::GetBoundingBox() const { return this->boundingBox; }

std::vector<SubMesh>& Mesh::GetSubMeshes() { return this->subMeshes; }

VertexBuffer& Mesh::GetVertexBuffer() { return this->vertexbuffer; }

IndexBuffer& Mesh::GetIndexBuffer() { return this->indexbuffer; }

void Mesh::SetIdentifier(std::string name) { this->identifier = name; }

size_t Mesh::GetMeshIndex() { return this->meshIndex; }

SubMesh::SubMesh(size_t startIndex, size_t nrOfIndices) : startIndex(startIndex), nrOfIndices(nrOfIndices) {}
SubMesh::~SubMesh() {}

size_t SubMesh::GetStartIndex() const { return this->startIndex; }

size_t SubMesh::GetNrOfIndices() const { return this->nrOfIndices; }

void Mesh::Init(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes,
				DirectX::BoundingBox boundingBox) {
	this->vertexbuffer = std::move(vertexbuffer);
	this->indexbuffer = std::move(indexbuffer);
	this->subMeshes = std::move(submeshes);
	this->boundingBox = std::move(boundingBox);

	static size_t index = 0;
	this->meshIndex = index++;
	Logger::Log("Index: ", this->meshIndex);
}
