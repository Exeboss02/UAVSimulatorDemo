#pragma once

#include "core/input/abstractInput.h"

enum ControllerKeybinds {
	CONTROLLER_MOVE_FORWARD = DPAD_UP,
	CONTROLLER_MOVE_BACKWARD = DPAD_DOWN,
	CONTROLLER_MOVE_LEFT = DPAD_LEFT,
	CONTROLLER_MOVE_RIGHT = DPAD_RIGHT,

	CONTROLLER_JUMP = A_BUTTON,
	CONTROLLER_SPRINT = B_BUTTON,

	CONTROLLER_INTERACT = X_BUTTON,

	CONTROLLER_FULLSCREEN_TOGGLE = START,
	CONTROLLER_QUIT = BACK
};

class ControllerInput : public AbstractInput {
public:
	ControllerInput(DWORD index) : controllerIndex(index) {}

	std::array<float, 2> GetMovementVector() const override;
	std::array<float, 2> GetLookVector() const override;

	bool Jump() const override;
	bool Sprint() const override;
	bool LeftClick() const override;
	bool RightClick() const override;
	bool Interact() const override;
	bool FullscreenToggle() const override;
	bool Quit() const override;
	bool LeftDown() const override;
	bool RightDown() const override; 

	DWORD GetControllerIndex() const { return this->controllerIndex; }

private:
	DWORD controllerIndex;
};