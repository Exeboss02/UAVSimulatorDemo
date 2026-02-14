#pragma once

#include "core/input/abstractInput.h"
#include "core/input/controllerInput.h"
#include "core/input/keyboardInput.h"
#include "gameObjects/cameraObject.h"

class DebugCamera : public CameraObject {
public:
	DebugCamera() : controllerInput(DWORD(0)), rot{0.0f, 0.0f, 0.0f} {
	}

	KeyboardInput keyboardInput;
	ControllerInput controllerInput;

	virtual void Tick() override;	
	virtual void SaveToJson(nlohmann::json& data) override;

private:
	void shootRay();

	float rot[3];
};