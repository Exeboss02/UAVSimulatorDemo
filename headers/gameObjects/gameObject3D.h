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
	/// Get global position as an XMVECTOR
	/// </summary>
	/// <returns></returns>
	virtual DirectX::XMVECTOR GetGlobalPosition() const;

	/// <summary>
	/// Get global rotation as a quaternion, in an XMVECTOR
	/// </summary>
	/// <returns></returns>
	virtual DirectX::XMVECTOR GetGlobalRotation() const;

	/// <summary>
	/// Get global scale as an XMVECTOR
	/// </summary>
	/// <returns></returns>
	virtual DirectX::XMVECTOR GetGlobalScale() const;

	/// <summary>
	/// Get XMMATRIX for the global world matrix
	/// </summary>
	/// <param name="inverseTranspose"></param>
	/// <returns></returns>
	DirectX::XMMATRIX GetGlobalWorldMatrix(bool inverseTranspose) const override;

	/// <summary>
	/// Maybe misleading name, but returns the forward vector of the object in global space
	/// </summary>
	/// <returns></returns>
	virtual DirectX::XMVECTOR GetGlobalForward() const;

	/// <summary>
	/// Maybe misleading name, but returns the right vector of the object in global space
	/// </summary>
	/// <returns></returns>
	virtual DirectX::XMVECTOR GetGlobalRight() const;

	/// <summary>
	/// Maybe misleading name, but returns the up vector of the object in global space
	/// </summary>
	/// <returns></returns>
	virtual DirectX::XMVECTOR GetGlobalUp() const;

	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	virtual void ShowInHierarchy() override;

private:
	enum TransformComponent { SCALE, ROTATION, TRANSLATAION };

	DirectX::XMVECTOR GetDecomposedWorldMatrix(const TransformComponent& component) const;
};