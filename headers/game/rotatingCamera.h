#pragma once
#include "gameObjects/cameraObject.h"

class RotatingCamera : CameraObject {
public:
	void Tick() override;
};