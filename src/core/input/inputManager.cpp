#include "core/input/inputManager.h"
#include <windowsx.h>

#include "utilities/logger.h"

InputManager::InputManager()
	: keyStates{},
	  controllers{{Controller(DWORD(0)), Controller(DWORD(1)), Controller(DWORD(2)), Controller(DWORD(3))}},
	  mousePosition{0, 0}, mouseMovement{0, 0}, LM{0}, RM{0} {}

void InputManager::SetKeyState(const unsigned char key, const unsigned char state) { keyStates[key] = state; }

void InputManager::SetLMouseKeyState(const unsigned char state) { this->LM = state; }

void InputManager::SetRMouseKeyState(const unsigned char state) { this->RM = state; }

void InputManager::SetMousePosition(const int x, const int y, bool reCenter) {

	if (reCenter) {
		this->mousePosition = {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
		return;
	}

	this->mouseMovement = {x - static_cast<int>(this->mousePosition.at(0)),
						   y - static_cast<int>(this->mousePosition.at(1))};
	this->mousePosition = {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
}

void InputManager::Reset() {
	for (unsigned char& state : keyStates) {
		state &= KEY_DOWN;
	}
	this->mouseMovement = {0, 0};
	this->LM &= KEY_DOWN;
	this->RM &= KEY_DOWN;
}

bool InputManager::ReadMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	this->currentWindowHandle = hWnd;
	const unsigned char key = static_cast<unsigned char>(wParam);
	switch (message) {

	/// Handle keyboard events
	case WM_KEYDOWN: {
		const bool wasDown = lParam & (1 << 30);
		if (!wasDown) this->SetKeyState(key, KEY_DOWN | KEY_PRESSED);
		return 0;
	}

	case WM_KEYUP: {
		this->SetKeyState(key, KEY_RELEASED);
		return 0;
	}

	/// Handle mouse events
	case WM_MOUSEMOVE: {
		const int xPos = GET_X_LPARAM(lParam);
		const int yPos = GET_Y_LPARAM(lParam);
		this->SetMousePosition(xPos, yPos);
		return 0;
	}

	case WM_LBUTTONDOWN: {
		if (!this->IsLMDown()) this->SetLMouseKeyState(KEY_DOWN | KEY_PRESSED);
		return 0;
	}

	case WM_LBUTTONUP: {
		this->SetLMouseKeyState(KEY_RELEASED);
		return 0;
	}

	case WM_RBUTTONDOWN: {
		if (!this->IsRMDown()) this->SetRMouseKeyState(KEY_DOWN | KEY_PRESSED);
		return 0;
	}

	case WM_RBUTTONUP: {
		this->SetRMouseKeyState(KEY_RELEASED);
		return 0;
	}

	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}

	default:
		return 1;
	}
}

std::array<int, 2> InputManager::GetMouseMovement() const {

	if (!this->currentWindowHandle) {
		return this->mouseMovement;
	}

	const bool isWindowFocused = GetForegroundWindow() == this->currentWindowHandle;
	const bool isWinDown = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
	const bool isShiftDown = GetAsyncKeyState(VK_SHIFT) & 0x8000;
	const bool isSDown = GetAsyncKeyState('S') & 0x8000;
	const bool isSnippingShortcutActive = isWinDown && isShiftDown && isSDown;

	// Avoid forcing the cursor to center while OS overlays (e.g. Win+Shift+S snip)
	// are active or when the game window is not focused.
	if (!isWindowFocused || isSnippingShortcutActive) {
		return this->mouseMovement;
	}

	RECT windowRect;
	GetClientRect(this->currentWindowHandle, &windowRect); // Get the window size

	POINT centerPoint = {(windowRect.right - windowRect.left) / 2, (windowRect.bottom - windowRect.top) / 2};

	// Convert to screen coordinates for SetCursorPos
	POINT screenCenter = centerPoint;
	ClientToScreen(this->currentWindowHandle, &screenCenter);
	SetCursorPos(screenCenter.x, screenCenter.y);

	// Store CLIENT coordinates (not screen coordinates!)
	const_cast<InputManager*>(this)->SetMousePosition(centerPoint.x, centerPoint.y, true);

	return this->mouseMovement;
}

std::array<int, 2> InputManager::GetMousePosition() const {
	return {static_cast<int>(this->mousePosition[0]), static_cast<int>(this->mousePosition[1])};
}

bool InputManager::IsKeyDown(const unsigned char key) const { return keyStates[key] & KEY_DOWN; }

bool InputManager::WasKeyPressed(const unsigned char key) const { return keyStates[key] & KEY_PRESSED; }

bool InputManager::WasKeyReleased(const unsigned char key) const { return keyStates[key] & KEY_RELEASED; }

bool InputManager::IsLMDown() const { return this->LM & KEY_DOWN; }

bool InputManager::WasLMPressed() const { return this->LM & KEY_PRESSED; }

bool InputManager::WasLMReleased() const { return this->LM & KEY_RELEASED; }

bool InputManager::IsRMDown() const { return this->RM & KEY_DOWN; }

bool InputManager::WasRMPressed() const { return this->RM & KEY_PRESSED; }

bool InputManager::WasRMReleased() const { return this->RM & KEY_RELEASED; }

std::vector<Controller> InputManager::GetConnectedControllers() {
	std::vector<Controller> connectedControllers;
	for (auto& controller : this->controllers) {
		if (controller.IsConnected()) {
			connectedControllers.push_back(controller);
		}
	}
	return connectedControllers;
}

const size_t InputManager::GetConnectedControllerCount() { return this->GetConnectedControllers().size(); }

void InputManager::ReadControllerInputs() {

	std::vector<Controller> connectedControllers = this->GetConnectedControllers();
	for (DWORD i = 0; i < connectedControllers.size(); ++i) {
		connectedControllers[i].ReadInput();
	}
}

void InputManager::ReadControllerInput(DWORD index) {
	if (index < XUSER_MAX_COUNT) {
		if (this->controllers[index].IsConnected()) {
			this->controllers[index].ReadInput();
		}
	}
}

std::array<float, 2> InputManager::GetLeftThumbMovement(DWORD index) const {
	return this->controllers[index].GetInput().leftThumb;
}

std::array<float, 2> InputManager::GetRightThumbMovement(DWORD index) const {
	return this->controllers[index].GetInput().rightThumb;
}

bool InputManager::IsLeftBackTriggerDown(DWORD index) const {
	return this->controllers[index].GetInput().leftBackTrigger;
}

bool InputManager::IsRightBackTriggerDown(DWORD index) const {
	return this->controllers[index].GetInput().rightBackTrigger;
}

bool InputManager::WasLeftBackTriggerPressed(DWORD index) const {
	return this->controllers[index].GetInput().leftBackTriggerPressed;
}

bool InputManager::WasRightBackTriggerPressed(DWORD index) const {
	return this->controllers[index].GetInput().rightBackTriggerPressed;
}

bool InputManager::IsControllerButtonDown(DWORD index, const int button) const {
	return (this->controllers[index].GetInput().buttons & button) != 0;
}

bool InputManager::WasControllerButtonPressed(DWORD index, const int button) const {
	return (this->controllers[index].GetInput().buttonsPressed & button) != 0;
}

bool InputManager::WasControllerButtonReleased(DWORD index, const int button) const {
	return (this->controllers[index].GetInput().buttonsReleased & button) != 0;
}