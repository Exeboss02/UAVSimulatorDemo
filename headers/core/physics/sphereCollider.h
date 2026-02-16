#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "core/physics/collider.h"

//sphere collider transform scale has to be unifrom!

class BoxCollider;

class SphereCollider : public Collider
{
public:
	SphereCollider();
	~SphereCollider();

	void Tick() override;
	void LoadFromJson(const nlohmann::json& data) override;
	void SaveToJson(nlohmann::json& data) override;

	/// <summary>
	/// ONLY USED BY ENGINE - Starts double dispatch call chain to check and resolve collisions automatically based on collision case
	/// </summary>
	/// <param name="otherCollider"></param>
	/// <param name="mtvAxis"></param>
	/// <param name="mtvDistance"></param>
	/// <returns></returns>
	bool DoubleDispatchCollision(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance) override;

	/// <summary>
	/// Sets position
	/// </summary>
	/// <param name="newCenter"></param>
	void SetPosition(DirectX::XMFLOAT3 newCenter) override;
	void SetRotation(DirectX::XMFLOAT3 newRotation);

	/// <summary>
	/// Sets the diameter
	/// </summary>
	/// <param name="diameter"></param>
	void SetDiameter(float diameter);

	/// <summary>
	/// Retrieves the diameter
	/// </summary>
	/// <returns></returns>
	float GetDiameter();

	bool IntersectWithRay(const Ray& ray, float& distance, float maxDistance) override;

private:
	bool CollidesWithBox(BoxCollider* box, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance) override;
	bool CollidesWithSphere(SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance) override;
};