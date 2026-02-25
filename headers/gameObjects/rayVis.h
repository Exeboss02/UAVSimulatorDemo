#pragma once
#include "utilities/time.h"
#include "gameObjects/meshObject.h"

class RayVis : public MeshObject {
public:
	RayVis();
	~RayVis();

	void StartDeathTimer(float timeToDeath);

	void Tick() override;

private:
	float timeOfCreation;
	float timeToDie = -1.0f;

};
