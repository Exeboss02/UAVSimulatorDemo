#pragma once

#include "core/assetManager.h"
#include "gameObjects/gameObject3D.h"
#include "gameObjects/mesh.h"
#include "gameObjects/meshObjData.h"
#include "rendering/renderQueue.h"

class MeshObject : public GameObject3D {
public:
	MeshObject();
	virtual ~MeshObject() = default;

	/// <summary>
	/// Update the mesh and add the object to the render queue
	/// </summary>
	/// <param name="newMesh"></param>
	void SetMesh(MeshObjData newMesh);

	MeshObjData GetMesh();

	virtual void Tick() override;

	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

private:
	MeshObjData mesh;

	int tempId;
};