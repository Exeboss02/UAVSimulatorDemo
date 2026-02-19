#pragma once

#include "gameObjects/meshObject.h"

class Turret : public MeshObject {
public:
	virtual void Start() override;
	virtual void Tick() override;


private:
	
};