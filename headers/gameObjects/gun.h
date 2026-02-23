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


private:

	std::weak_ptr<MeshObject> gunVisual;

	std::vector<SoundClip*> soundClips;
	std::weak_ptr<SoundSourceObject> speaker;
	std::weak_ptr<GameObject3D> muzzle;

	Timer musicTimer;
	Timer sfxTimer;
	Timer shootCoolDown;

	float damage = 20;
};
