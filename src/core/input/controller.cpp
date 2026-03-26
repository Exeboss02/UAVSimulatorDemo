#include "core/input/controller.h"

#include <cmath>

#include "utilities/logger.h"

static constexpr unsigned int leftThumbDeadzone = 2000;
static constexpr unsigned int rightThumbDeadzone = 2000;
static constexpr unsigned int triggerDeadzone = 30;

static constexpr float maxThumbMagnitude = 32767.0f;
static constexpr float maxTriggerMagnitude = 255.0f; // May not be needed

const bool Controller::IsConnected() {
	this->previousState = this->state;
	
	DWORD result = XInputGetState(this->controllerIndex, &this->state);
	
	if (result == ERROR_SUCCESS) {
		return true;
	}
	
	return false;
}

void Controller::ReadInput()
{
	XINPUT_GAMEPAD gamepad = this->state.Gamepad;
	XINPUT_GAMEPAD previousGamepad = this->previousState.Gamepad;
	
	// Track button state changes
	WORD previousButtons = previousGamepad.wButtons;
	WORD currentButtons = gamepad.wButtons;
	
	this->input.buttons = currentButtons;
	this->input.buttonsPressed = (currentButtons & ~previousButtons);  // Newly pressed
	this->input.buttonsReleased = (~currentButtons & previousButtons);  // Newly released

	// Deadzone LEFT STICK
	float LX = gamepad.sThumbLX;
	float LY = gamepad.sThumbLY;

	float leftMagnitude = sqrt(LX * LX + LY * LY);
	float normMagnitude = leftMagnitude / (maxThumbMagnitude - leftThumbDeadzone);

	float normLX = LX / leftMagnitude;
	float normLY = LY / leftMagnitude;

	if (leftMagnitude > leftThumbDeadzone) {
		if(normMagnitude > 1.0f) 
			normMagnitude = 1.0f;

		LX = normLX * normMagnitude;
		LY = normLY * normMagnitude;
	} 
	else {
		LX = 0.0f;
		LY = 0.0f;
	}

	this->input.leftThumb[0] = LX;
	this->input.leftThumb[1] = LY;
	
	// Deadzone RIGHT STICK
	float RX = gamepad.sThumbRX;
	float RY = gamepad.sThumbRY;

	float rightMagnitude = sqrt(RX * RX + RY * RY);
	normMagnitude = rightMagnitude / (maxThumbMagnitude - rightThumbDeadzone);

	float normRX = RX / rightMagnitude;
	float normRY = RY / rightMagnitude;

	if (rightMagnitude > rightThumbDeadzone) {
		if (normMagnitude > 1.0f)
			normMagnitude = 1.0f;

		RX = normRX * normMagnitude;
		RY = normRY * normMagnitude;
	}
	else {
		RX = 0.0f;
		RY = 0.0f;
	}

	this->input.rightThumb[0] = RX;
	this->input.rightThumb[1] = RY;

	// Deadzone BACK TRIGGERS ( ON / OFF )
	bool previousLT = previousGamepad.bLeftTrigger > triggerDeadzone;
	bool previousRT = previousGamepad.bRightTrigger > triggerDeadzone;
	
	float LT = gamepad.bLeftTrigger;
	if (LT > triggerDeadzone) {
		LT = 1.f;
	}
	else {
		LT = 0.0f;
	}

	this->input.leftBackTrigger = bool(LT);
	this->input.leftBackTriggerPressed = this->input.leftBackTrigger && !previousLT;
	this->input.leftBackTriggerReleased = !this->input.leftBackTrigger && previousLT;

	float RT = gamepad.bRightTrigger;
	if (RT > triggerDeadzone) {
		RT = 1.f;
	}
	else {
		RT = 0.0f;
	}
	this->input.rightBackTrigger = bool(RT);
	this->input.rightBackTriggerPressed = this->input.rightBackTrigger && !previousRT;
	this->input.rightBackTriggerReleased = !this->input.rightBackTrigger && previousRT;
}

const RawControllerInput& Controller::GetInput() const { return this->input; }

void Controller::PrintInputState() {
	Logger::Log("Left Thumb: " + std::to_string(this->input.leftThumb[0]) + std::to_string(this->input.leftThumb[1]));
	Logger::Log("Right Thumb: " + std::to_string(this->input.rightThumb[0]) + std::to_string(this->input.rightThumb[1]));
	Logger::Log("Left Back Trigger" + std::to_string(this->input.leftBackTrigger));
	Logger::Log("Right Back Trigger" + std::to_string(this->input.rightBackTrigger));

	Logger::Log("Buttons bitmask: " + std::to_string(this->input.buttons));
}