#pragma once 
#include "gameObjects/gameObject3D.h"
#include "gameObjects/meshObject.h"
#include "core/tools.h"
#include "core/physics/physicsQueue.h"

class Gun : public GameObject3D {
public:
	Gun();
	~Gun();

	void Shoot();

	void Tick() override;

	void Start() override;

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


private:

	std::weak_ptr<GameObject3D> parentCamera;

	std::weak_ptr<MeshObject> gunVisual;

	std::vector<SoundClip*> soundClips;
	std::weak_ptr<SoundSourceObject> speaker;
	std::weak_ptr<GameObject3D> muzzle;
	Timer shootCoolDown;
};
