#include "gameObjects/mesh.h"

Mesh::Mesh(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes) 
    : vertexbuffer(std::move(vertexbuffer)), indexbuffer(std::move(indexbuffer)), subMeshes(std::move(submeshes)) {
}

Mesh::~Mesh() {

}

std::string& Mesh::GetIdentifier()
{
    return this->identifier;
}

std::vector<SubMesh>& Mesh::GetSubMeshes()
{
    return this->subMeshes;
}

VertexBuffer& Mesh::GetVertexBuffer()
{
    return this->vertexbuffer;
}

IndexBuffer& Mesh::GetIndexBuffer()
{
    return this->indexbuffer;
}

void Mesh::SetIdentifier(std::string name)
{
    this->identifier = name;
}

SubMesh::SubMesh(size_t startIndex, size_t nrOfIndices)
    : startIndex(startIndex), nrOfIndices(nrOfIndices) {

}
SubMesh::~SubMesh() {

}

size_t SubMesh::GetStartIndex() const
{
    return this->startIndex;
}

size_t SubMesh::GetNrOfIndices() const
{
    return this->nrOfIndices;
}

void Mesh::Init(VertexBuffer vertexbuffer, IndexBuffer indexbuffer, std::vector<SubMesh>&& submeshes) {
    this->vertexbuffer = std::move(vertexbuffer);
	this->indexbuffer = std::move(indexbuffer);
	this->subMeshes = std::move(submeshes);
}
