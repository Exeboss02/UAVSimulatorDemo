#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "core/physics/collider.h"

//axis is never updated => normals never updated?


class SphereCollider;

class BoxCollider : public Collider
{
public:
	BoxCollider();
	~BoxCollider();

	void Tick() override;
	void LoadFromJson(const nlohmann::json& data) override;
	void SaveToJson(nlohmann::json& data) override;

	/// <summary>
	/// ONLY USED BY ENGINE - Starts the double dispatch call chain to automatically determine collision case
	/// </summary>
	/// <param name="otherCollider"></param>
	/// <param name="mtvAxis"></param>
	/// <param name="mtvDistance"></param>
	/// <returns></returns>
	bool DoubleDispatchCollision(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance) override;

	/// <summary>
	/// Sets position and updates SAT-data
	/// </summary>
	/// <param name="newCenter"></param>
	void SetPosition(DirectX::XMFLOAT3 newCenter) override;

	/// <summary>
	/// Sets rotation and updates SAT-data
	/// </summary>
	/// <param name="newRotation"></param>
	void SetRotation(DirectX::XMFLOAT3 newRotation);

	/// <summary>
	/// Sets extents, a float3 with lengths from center of box to each side
	/// </summary>
	/// <param name="newSize"></param>
	void SetExtents(DirectX::XMFLOAT3 newSize);

	/// <summary>
	/// Retrieve extents float3
	/// </summary>
	/// <returns></returns>
	DirectX::XMFLOAT3 GetExtents();

	bool IntersectWithRay(const Ray& ray, float& distance, float maxDistance) override;

private:
	DirectX::XMFLOAT3 axis[3];
	SATData satData = {};

	bool CollidesWithBox(BoxCollider* box, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance) override;
	bool CollidesWithSphere(SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance) override;
	void BuildCornersArray(DirectX::XMFLOAT3*& positionArray);
};