#pragma once

#include "gameObjects/gameObject.h"
#include "gameObjects/transform.h"

class GameObject3D : public GameObject {
public:
	GameObject3D();
	virtual ~GameObject3D() = default;

	virtual void Tick() override;

	Transform transform;

	/// <summary>
	/// Engine only. Don't use in game. Gets global world matrix for this object.
	/// </summary>
	DirectX::XMMATRIX GetGlobalWorldMatrixRecursive(bool inverseTranspose) const override;

	/// <summary>
	/// Engine only. Tells children that they have moved.
	/// </summary>
	void SetHasMovedRecursive() override;


	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	virtual void ShowInHierarchy() override;

private:
};