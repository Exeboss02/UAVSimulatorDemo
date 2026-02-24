#pragma once 
#include "gameObjects/gameObject3D.h"

class Cockpit : public GameObject3D {
public:
	void Start() override;

private:
	float health = 10;
};