#include "gameObjects/meshObjData.h"
#include "utilities/logger.h"


void MeshObjData::SetMesh(std::shared_ptr<Mesh> mesh) {
	this->mesh = std::weak_ptr<Mesh>(mesh);
	this->materials.clear();
	this->materials.insert(this->materials.end(), mesh->GetSubMeshes().size(), std::weak_ptr<BaseMaterial>());
}; 

void MeshObjData::SetMaterial(size_t index, std::shared_ptr<BaseMaterial> material) {
	if (index >= this->materials.size()) {
		Logger::Error("Trying to set material out of bounds, doing nothing, FILE: ", __FILE__, ", LINE: ", __LINE__);
		return;
	}

	this->materials.at(index) = material;
};
std::string MeshObjData::GetMeshIdentifier() {
	if (!this->mesh.expired()) {
		return this->mesh.lock()->GetIdentifier();
	}
	else {
		Logger::Warn("Called 'GetMeshIdentifier' but mesh has expired");
		return "";
	}
}
std::weak_ptr<Mesh> MeshObjData::GetMesh() {
	return this->mesh;
}
std::optional<SubMeshData> MeshObjData::GetSubMeshData(size_t index) {
	if (this->mesh.expired()) {
		return std::make_optional<SubMeshData>();
	}
	std::vector<SubMesh>& submeshes = this->mesh.lock()->GetSubMeshes();
	if (index >= submeshes.size()) {
		return std::make_optional<SubMeshData>();
	}
	
	SubMeshData data;
	data.submesh = submeshes.at(index);
	data.material = this->GetMaterial(index);

	return std::make_optional<SubMeshData>(std::move(data));
}
std::weak_ptr<BaseMaterial> MeshObjData::GetMaterial(size_t index)
{
	if (index >= this->materials.size()) {
		Logger::Error("Trying to get Material out of bounds");
		return std::weak_ptr<BaseMaterial>();
	}
	return this->materials.at(index);
}
