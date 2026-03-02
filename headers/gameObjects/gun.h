#pragma once 
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/tools.h"
#include "core/physics/physicsQueue.h"

class Gun : public GameObject3D {
public:
	Gun();
	~Gun();

	void Shoot(bool pressedTrigger, bool triggerDown);

	void Tick() override;

	void Start() override;

	void setParentToShootFrom(std::shared_ptr<GameObject3D> parentCamera);

protected:

	std::string gunVisualPath = "";
	std::string soundClipShootPath = "";
	float damage = 20;
	float fireRate = 0.3f;
	DirectX::XMFLOAT3 muzzlePosition = DirectX::XMFLOAT3(0,0,0);
	DirectX::XMFLOAT3 visualPosition = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 visualScale = DirectX::XMFLOAT3(1, 1, 1);
	DirectX::XMFLOAT3 visualRotationRPY = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 gunPosition = DirectX::XMFLOAT3(0, 0, 0);
	enum FireMode {
		SEMIAUTOMATIC, 
		AUTOMATIC
	}fireMode;
	

private:

	std::weak_ptr<GameObject3D> parentCamera;

	std::weak_ptr<MeshObject> gunVisual;

	std::weak_ptr<SoundSourceObject> speaker;
	std::weak_ptr<GameObject3D> muzzle;
	Timer shootCoolDown;
	/// <summary>
	/// used when the gun shoots from the camera origin
	/// </summary>
	void VisualizeShootBasedOnCameraParent(DirectX::XMVECTOR lookVec, DirectX::XMVECTOR posVec, float distance);
	/// <summary>
	/// used when gun shoots from muzzle
	/// </summary>
	void VisulalizeShootBasedOnMuzzle(DirectX::XMVECTOR lookVec, DirectX::XMVECTOR posVec, float distance);

};
