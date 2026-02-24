#pragma once

#include "core/input/abstractInput.h"

enum KeyboardKeybinds {
	MOVE_FORWARD = 'W',
	MOVE_BACKWARD = 'S',
	MOVE_LEFT = 'A',
	MOVE_RIGHT = 'D',
	TOGGLE_CAMERA = 'O',
	JUMP = VK_SPACE,
	SPRINT = VK_LSHIFT,

	LEFT_CLICK = VK_LBUTTON,
	RIGHT_CLICK = VK_RBUTTON,

	INTERACT = 'F',

	FULLSCREEN_TOGGLE = VK_F11,
	QUIT = VK_ESCAPE,
};

class KeyboardInput : public AbstractInput {
public:
	KeyboardInput() = default;

	std::array<float, 2> GetMovementVector() const override;
	std::array<float, 2> GetLookVector() const override;
	bool Jump() const override;
	bool Sprint() const override;
	bool LeftClick() const override;
	bool RightClick() const override;
	bool Interact() const override;
	bool FullscreenToggle() const override;
	bool Quit() const override;
	bool ToggleCamera() const;

};