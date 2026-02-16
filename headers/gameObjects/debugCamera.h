#pragma once

#include "core/input/abstractInput.h"
#include "core/input/controllerInput.h"
#include "core/input/keyboardInput.h"
#include "gameObjects/cameraObject.h"

class DebugCamera : CameraObject {
public:
	DebugCamera() : controllerInput(DWORD(0)) {}

	KeyboardInput keyboardInput;
	ControllerInput controllerInput;

	virtual void Tick() override;
};