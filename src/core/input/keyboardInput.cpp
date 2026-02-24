#include "core/input/keyboardInput.h"

std::array<float, 2> KeyboardInput::GetMovementVector() const
{
    float x = 0.0f;
    float y = 0.0f;

    if (InputManager::GetInstance().IsKeyDown(MOVE_FORWARD)) {
        y += 1.0f;
    }
    if (InputManager::GetInstance().IsKeyDown(MOVE_BACKWARD)) {
        y -= 1.0f;
    }
    if (InputManager::GetInstance().IsKeyDown(MOVE_LEFT)) {
        x -= 1.0f;
    }
    if (InputManager::GetInstance().IsKeyDown(MOVE_RIGHT)) {
        x += 1.0f;
    }

    // Normalize the vector
    float magnitude = sqrt(x * x + y * y);
    if (magnitude > 0.0f) {
        x /= magnitude;
        y /= magnitude;
    }
	return { x, y };
}

std::array<float, 2> KeyboardInput::GetLookVector() const
{
    // Mouse movement for looking around
    std::array<int, 2> mouseMovement = InputManager::GetInstance().GetMouseMovement();

    float lookX = static_cast<float>(mouseMovement[0]);
    float lookY = static_cast<float>(mouseMovement[1]);

    //// Normalize the look vector
    //float magnitude = sqrt(lookX * lookX + lookY * lookY);
    //if (magnitude > 0.0f) {
    //    lookX /= magnitude;
    //    lookY /= magnitude;
    //}
	return { lookX, lookY };
}

bool KeyboardInput::Jump() const { return InputManager::GetInstance().WasKeyPressed(JUMP); }

bool KeyboardInput::Sprint() const { return InputManager::GetInstance().IsKeyDown(SPRINT); }

bool KeyboardInput::LeftClick() const { return InputManager::GetInstance().WasLMPressed(); }

bool KeyboardInput::RightClick() const { return InputManager::GetInstance().WasRMPressed(); }

bool KeyboardInput::Interact() const { return InputManager::GetInstance().WasKeyPressed(INTERACT); }

bool KeyboardInput::FullscreenToggle() const { return InputManager::GetInstance().WasKeyPressed(FULLSCREEN_TOGGLE); }

bool KeyboardInput::Quit() const { return InputManager::GetInstance().WasKeyPressed(QUIT); }

bool KeyboardInput::ToggleCamera() const { return InputManager::GetInstance().WasKeyPressed(TOGGLE_CAMERA); }