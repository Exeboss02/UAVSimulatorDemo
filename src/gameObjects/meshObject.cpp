#include "gameObjects/meshObject.h"

MeshObject::MeshObject() : mesh(), imguiNewMeshIdent("\0"), imguiNewMatIdent("\0"), hide(false) {
	static int id = 0;
	this->tempId = id++;
	Logger::Log("Created a MeshObject.");
}

void MeshObject::SetMesh(MeshObjData newMesh) {
	this->mesh = newMesh;

	// Should do a check to make sure it isn't already in render queue
}

MeshObjData& MeshObject::GetMesh() { return this->mesh; }

DirectX::BoundingBox MeshObject::GetBoundingBox() {
	auto meshWeak = this->mesh.GetMesh();
	if (meshWeak.expired()) {
		std::string error = "Trying to get bounding box from a MeshObject without a mesh or with a dead mesh";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	auto box = meshWeak.lock()->GetBoundingBox();
	DirectX::XMMATRIX matrix = this->GetGlobalWorldMatrix(false);
	box.Transform(box, matrix);
	return box;
}

void MeshObject::Tick() {
	GameObject3D::Tick();

	//Logger::Log("Mesh");
	//Logger::Log(this->GetGlobalPosition().m128_f32[0], ":", this->GetBoundingBox().Center.x);
	//Logger::Log(this->GetGlobalPosition().m128_f32[1], ":", this->GetBoundingBox().Center.y);
	//Logger::Log(this->GetGlobalPosition().m128_f32[2], ":", this->GetBoundingBox().Center.z);

}

void MeshObject::LateTick() { 
	this->cachedGlobalPosition = this->GetGlobalPosition();
	this->cachedGlobalMatrix = this->GetGlobalWorldMatrix(false);
	this->cachedGlobalMatrixIT = this->GetGlobalWorldMatrix(true);
}

void MeshObject::Start() { 
	this->GameObject3D::Start();
	RenderQueue::AddMeshObject(this->GetPtr());
}

void MeshObject::LoadFromJson(const nlohmann::json& data) {
	this->GameObject3D::LoadFromJson(data);

	if (data.contains("meshIdentifier")) {
		SetMesh(AssetManager::GetInstance().GetMeshObjData(data["meshIdentifier"].get<std::string>()));
	}
}

void MeshObject::SaveToJson(nlohmann::json& data) {
	this->GameObject3D::SaveToJson(data);

	data["type"] = "MeshObject";

	data["meshIdentifier"] = GetMesh().GetMeshIdentifier();
}

void MeshObject::ShowInHierarchy() {
	this->GameObject3D::ShowInHierarchy();

	ImGui::Text("MeshObject");

	ImGui::Checkbox("Hide", &this->hide);

	if (!this->GetMesh().GetMesh().expired()) {
		std::string meshText = std::format("Mesh: {}", this->GetMesh().GetMeshIdentifier());
		ImGui::Text(meshText.c_str());

		ImGui::Text("Materials");
		for (int i = 0; i < this->GetMesh().GetMesh().lock()->GetSubMeshes().size(); i++) {
			std::string materialName = this->GetMesh().GetMaterial(i).lock()->GetIdentifier();
			std::string shortMaterialName = materialName;
			if (shortMaterialName.size() > 32) {
				shortMaterialName = "..." + shortMaterialName.substr(shortMaterialName.size() - 32, std::string::npos);
			}

			shortMaterialName = std::to_string(i) + ". " + shortMaterialName;

			if (ImGui::TreeNode(shortMaterialName.c_str())) {
				ImGui::Text(("Identifier: " + materialName).c_str());

				if (ImGui::Button("Change material")) ImGui::OpenPopup("change_mat");
				if (ImGui::BeginPopup("change_mat")) {
					ImGui::InputText("New Material", this->imguiNewMatIdent, sizeof(this->imguiNewMatIdent));
					if (ImGui::Button("Apply")) {
						Logger::Log("Tried to change material.");
						this->GetMesh().SetMaterial(
							i, AssetManager::GetInstance().GetMaterialWeakPtr(this->imguiNewMatIdent).lock());
						std::strncpy(this->imguiNewMatIdent, "", sizeof(this->imguiNewMatIdent));
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::TreePop();
			};
		}
	}

	if (ImGui::Button("Change mesh")) ImGui::OpenPopup("change_mesh");
	if (ImGui::BeginPopup("change_mesh")) {
		ImGui::InputText("New Mesh", this->imguiNewMeshIdent, sizeof(this->imguiNewMeshIdent));
		/*if (!std::filesystem::exists(FilepathHolder::GetAssetsDirectory() / this->imguiNewMeshIdent))
			ImGui::Text("Invalid file.");*/
		if (ImGui::Button("Apply")) {
			Logger::Log("Tried to change mesh.");
			this->SetMesh(AssetManager::GetInstance().GetMeshObjData(this->imguiNewMeshIdent));
			std::strncpy(this->imguiNewMeshIdent, "", sizeof(this->imguiNewMeshIdent));
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

bool MeshObject::IsHidden() { return this->hide; }

DirectX::XMVECTOR& MeshObject::GetCachedGlobalPosition() { return this->cachedGlobalPosition; }

DirectX::XMMATRIX& MeshObject::GetCachedGlobalMatrix(bool inverseTransposed) { 
	if (inverseTransposed) {
		return this->cachedGlobalMatrixIT; 
	}
	else
	{
		return this->cachedGlobalMatrix;
	}
}
