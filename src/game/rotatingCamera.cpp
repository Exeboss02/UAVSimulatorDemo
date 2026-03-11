#include "game/rotatingCamera.h"

void RotatingCamera::Tick() { this->transform.Rotate(0, 0.05f * Time::GetInstance().GetDeltaTime(), 0); }
