#include "core/input/controllerInput.h"

std::array<float, 2> ControllerInput::GetMovementVector() const
{
	std::array<float, 2> movement = InputManager::GetInstance().GetLeftThumbMovement(this->controllerIndex);

	// If the left thumbstick is not being used, check the D-pad
	if (movement[0] == 0.0f && movement[1] == 0.0f) {
		if (InputManager::GetInstance().IsControllerButtonDown(this->controllerIndex, static_cast<int>(CONTROLLER_MOVE_FORWARD))) {
			movement[1] += 1.0f;
		}
		if (InputManager::GetInstance().IsControllerButtonDown(this->controllerIndex, static_cast<int>(CONTROLLER_MOVE_BACKWARD))) {
			movement[1] -= 1.0f;
		}
		if (InputManager::GetInstance().IsControllerButtonDown(this->controllerIndex, static_cast<int>(CONTROLLER_MOVE_LEFT))) {
			movement[0] -= 1.0f;
		}
		if (InputManager::GetInstance().IsControllerButtonDown(this->controllerIndex, static_cast<int>(CONTROLLER_MOVE_RIGHT))) {
			movement[0] += 1.0f;
		}
	}

	// normalize the vector 
	if (movement[0] != 0.0f || movement[1] != 0.0f) {
		float magnitude = sqrt(movement[0] * movement[0] + movement[1] * movement[1]);
		movement[0] /= magnitude;
		movement[1] /= magnitude;
	}

	return movement;
}

std::array<float, 2> ControllerInput::GetLookVector() const
{
	return InputManager::GetInstance().GetRightThumbMovement(this->controllerIndex);
}

bool ControllerInput::Jump() const { return InputManager::GetInstance().WasControllerButtonPressed(this->controllerIndex, static_cast<int>(CONTROLLER_JUMP)); }

bool ControllerInput::Sprint() const { return InputManager::GetInstance().IsControllerButtonDown(this->controllerIndex, static_cast<int>(CONTROLLER_SPRINT)); }

bool ControllerInput::LeftClick() const { return InputManager::GetInstance().WasLeftBackTriggerPressed(this->controllerIndex); }

bool ControllerInput::RightClick() const {return InputManager::GetInstance().WasRightBackTriggerPressed(this->controllerIndex);}

bool ControllerInput::Interact() const { return InputManager::GetInstance().WasControllerButtonPressed(this->controllerIndex, static_cast<int>(CONTROLLER_INTERACT)); }

bool ControllerInput::FullscreenToggle() const { return InputManager::GetInstance().WasControllerButtonPressed(this->controllerIndex, static_cast<int>(CONTROLLER_FULLSCREEN_TOGGLE)); }

bool ControllerInput::Quit() const { return InputManager::GetInstance().WasControllerButtonPressed(this->controllerIndex, static_cast<int>(CONTROLLER_QUIT)); }
