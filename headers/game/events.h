#pragma once

enum class ButtonEvent : int {
	NONE = 0 << 0,
	PLAY = 1 << 0,
	EXIT = 1 << 1,
};