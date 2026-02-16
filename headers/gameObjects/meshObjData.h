#pragma once

#include "gameObjects/mesh.h"
#include "rendering/baseMaterial.h"
#include <optional>
#include <string>

#include <memory>

/// <summary>
/// Data to render a single Submesh, note that this still needs the Index and Vertex buffer from the Mesh
/// </summary>
struct SubMeshData {
	SubMesh submesh = SubMesh(0, 0);
	std::weak_ptr<BaseMaterial> material;
};

/// <summary>
/// Contains reference data needed to render a mesh, owns nothing.
/// </summary>
class MeshObjData {
public:
	MeshObjData() = default;
	MeshObjData(MeshObjData&&) noexcept = default;
	MeshObjData& operator=(MeshObjData&&) noexcept = default;
	MeshObjData(MeshObjData&) = default;
	MeshObjData& operator=(MeshObjData&) = default;

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMaterial(size_t index, std::shared_ptr<BaseMaterial> material);
	
	std::weak_ptr<Mesh> GetMesh();
	std::optional<SubMeshData> GetSubMeshData(size_t index);
	std::weak_ptr<BaseMaterial> GetMaterial(size_t index);

	std::string GetMeshIdentifier();

private:
	std::weak_ptr<Mesh> mesh;
	std::vector<std::weak_ptr<BaseMaterial>> materials;
};
