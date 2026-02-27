#pragma once

#include <array>

#include "core/input/inputManager.h"

/// <summary>
/// Abstract class for handling user input no matter keyboard or handheld controller.
/// </summary>
class AbstractInput {
public:
	virtual ~AbstractInput() = default;
	virtual std::array<float, 2> GetMovementVector() const = 0;
	virtual std::array<float, 2> GetLookVector() const = 0;
	virtual bool Jump() const = 0;
	virtual bool Sprint() const = 0;
	virtual bool LeftClick() const = 0;
	virtual bool RightClick() const = 0;
	virtual bool LeftDown() const = 0;
	virtual bool RightDown() const = 0;
	virtual bool Interact() const = 0;
	virtual bool FullscreenToggle() const = 0;
	virtual bool Quit() const = 0;
};