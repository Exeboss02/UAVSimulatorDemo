#pragma once

#include "gameObjects/meshObject.h"

class Turret : public MeshObject {
public:
	virtual Start() override;
	virtual Tick() override;


private:
	
};