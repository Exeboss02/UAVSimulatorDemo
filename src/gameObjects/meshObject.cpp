#include "gameObjects/meshObject.h"

MeshObject::MeshObject() : mesh(), imguiNewMeshIdent("\0"), imguiNewMatIdent("\0"), hide(false) {
	static int id = 0;
	this->tempId = id++;
	Logger::Log("Created a MeshObject.");
}

void MeshObject::SetMesh(MeshObjData newMesh)
{
	this->mesh = newMesh;

	// Should do a check to make sure it isn't already in render queue

	RenderQueue::AddMeshObject(this->GetPtr());
}

MeshObjData& MeshObject::GetMesh()
{
	return this->mesh;
}

void MeshObject::Tick()
{
	GameObject3D::Tick();

	//static float rot = 0;
	//this->transform.SetRotationRPY(0,0,rot += 0.0005f);

	// Debug stuff to delete objects
	//ImGui::Begin("Object" + this->tempId);
	//if (ImGui::Button("Add")) {
	//	auto newCam = this->factory->CreateGameObjectOfType<GameObject>();
	//}
	//if (ImGui::Button("Delete")) {
	//	this->factory->QueueDeleteGameObject(this->GetPtr());
	//}
	//ImGui::End();
}

void MeshObject::LoadFromJson(const nlohmann::json& data)
{
	this->GameObject3D::LoadFromJson(data);

	if (data.contains("meshIdentifier")) {
		SetMesh(AssetManager::GetInstance().GetMeshObjData(data["meshIdentifier"].get<std::string>()));
	}
}

void MeshObject::SaveToJson(nlohmann::json& data)
{
	this->GameObject3D::SaveToJson(data);

	data["type"] = "MeshObject";

	data["meshIdentifier"] = GetMesh().GetMeshIdentifier();
}

void MeshObject::ShowInHierarchy() 
{ 
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
				ImGui::Text(("Identifier: " +  materialName).c_str());

				if (ImGui::Button("Change material")) ImGui::OpenPopup("change_mat");
				if (ImGui::BeginPopup("change_mat")) {
					ImGui::InputText("New Material", this->imguiNewMatIdent, sizeof(this->imguiNewMatIdent));
					if (ImGui::Button("Apply")) {
						Logger::Log("Tried to change material.");
						this->GetMesh().SetMaterial(i, AssetManager::GetInstance().GetMaterialWeakPtr(this->imguiNewMatIdent).lock());
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
