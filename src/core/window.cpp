#include "core/window.h"
#include "imgui.h"
#include "utilities/logger.h"

// std
#include <exception>
#include <iostream>

// Static current client size accessible globally
UINT Window::currentWidth = 0;
UINT Window::currentHeight = 0;
UINT Window::currentDPI = 0;

LRESULT Window::StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_NCCREATE) {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		Window* window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		window->hWnd = hWnd;
		return window->WindowProc(hWnd, message, wParam, lParam);
	}

	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (window) {
		return window->WindowProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!DISABLE_IMGUI) {
		ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
	}

	if (message == WM_KEYDOWN) {
		if (wParam == VK_F11) {
			this->ToggleFullscreen(!this->isFullscreen);
			return TRUE;
		}
	} else if (message == WM_DESTROY) {
		// close the application entirely
		PostQuitMessage(0);
		return TRUE;
	} else if (message == WM_CLOSE) {
		PostQuitMessage(0);
		return TRUE;
	}

	bool result = InputManager::GetInstance().ReadMessage(hWnd, message, wParam, lParam);

	if (!result) {
		// Check for window resize
		if (message == WM_SIZE) {
			if (wParam != SIZE_MINIMIZED) {
				UINT clientWidth = LOWORD(lParam);
				UINT clientHeight = HIWORD(lParam);

				if (clientWidth > 0 && clientHeight > 0) {
					if (!this->isFullscreen) {
						this->width = clientWidth;
						this->height = clientHeight;

						UINT dpi = GetDpiForWindow(this->GetHWND()); 

						if (this->resizeCallback) {
							this->resizeCallback(this->width, this->height, dpi);
						}
					}
				}
			}
			return TRUE;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Window::UpdateClientSize() {
	RECT rc{};
	if (GetClientRect(this->hWnd, &rc)) {
		UINT clientWidth = static_cast<UINT>(rc.right - rc.left);
		UINT clientHeight = static_cast<UINT>(rc.bottom - rc.top);
		if (clientWidth > 0 && clientHeight > 0) {
			this->width = clientWidth;
			this->height = clientHeight;
			Window::currentWidth = this->width;
			Window::currentHeight = this->height;
		}
	}
}

void Window::ApplyFullscreenResolution(UINT width, UINT height) {
	if (width == 0 || height == 0) {
		return;
	}

	if (!this->hasOriginalDisplayMode) {
		DEVMODE currentMode{};
		currentMode.dmSize = sizeof(currentMode);
		if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &currentMode)) {
			this->originalDisplayMode = currentMode;
			this->hasOriginalDisplayMode = true;
		}
	}

	DEVMODE newMode{};
	newMode.dmSize = sizeof(newMode);
	newMode.dmPelsWidth = width;
	newMode.dmPelsHeight = height;

	if (this->hasOriginalDisplayMode) {
		newMode.dmBitsPerPel = this->originalDisplayMode.dmBitsPerPel;
		newMode.dmDisplayFrequency = this->originalDisplayMode.dmDisplayFrequency;
		newMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
	} else {
		newMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	}

	ChangeDisplaySettings(&newMode, CDS_FULLSCREEN);
}

Window::Window(const HINSTANCE instance, int nCmdShow, const std::string name, const UINT width, const UINT height)
	: instance(instance), width(width), height(height), hWnd(nullptr), isFullscreen(false),
	  hasOriginalDisplayMode(false) {

	const wchar_t CLASS_NAME[] = L"WINDOW_CLASS";
	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD exStyle = 0;

	// Disable manual resizing
	style &= ~WS_THICKFRAME;
	style &= ~WS_MAXIMIZEBOX;

	WNDCLASSEX wc = {.cbSize = sizeof(WNDCLASSEX),
					 .lpfnWndProc = Window::StaticWindowProc,
					 .hInstance = this->instance,
					 .hCursor = LoadCursor(nullptr, IDC_ARROW), // default cursor, fixes endless startapp cursor
					 .lpszClassName = CLASS_NAME};

	if (!RegisterClassEx(&wc)) {
		throw std::runtime_error("Failed to register window class, error: " + std::to_string(GetLastError()));
	}

	RECT desiredRect{0, 0, static_cast<LONG>(this->width), static_cast<LONG>(this->height)};
	if (!AdjustWindowRectEx(&desiredRect, style, FALSE, exStyle)) {
		throw std::runtime_error("Failed to adjust window rect, error: " + std::to_string(GetLastError()));
	}

	int windowWidth = desiredRect.right - desiredRect.left;
	int windowHeight = desiredRect.bottom - desiredRect.top;

	std::wstring widestr = std::wstring(name.begin(), name.end());
	const wchar_t* winName = widestr.c_str();

	this->hWnd = CreateWindowEx(0, CLASS_NAME, winName, style, CW_USEDEFAULT, exStyle, windowWidth, windowHeight,
								nullptr, nullptr, this->instance, this);

	if (!hWnd) {
		throw std::runtime_error("Failed to create window, error: " + std::to_string(GetLastError()));
	}

	this->Show(nCmdShow);
	this->Resize(this->width, this->height);
}

Window::~Window() {
	if (this->hWnd) {
		DestroyWindow(this->hWnd);
	}
	UnregisterClass(L"WINDOW_CLASS", this->instance);
}

HWND Window::GetHWND() const { return this->hWnd; }

UINT Window::GetWidth() const { return this->width; }

UINT Window::GetHeight() const { return this->height; }

bool Window::IsFullscreen() const { return this->isFullscreen; }

void Window::Show(int nCmdShow) {
	ShowWindow(this->hWnd, nCmdShow);
	UpdateWindow(this->hWnd);
	this->UpdateClientSize();
}

void Window::Resize(UINT width, UINT height) {
	if (width == 0 || height == 0) {
		return;
	}
	this->width = width;
	this->height = height;
	Window::currentWidth = this->width;
	Window::currentHeight = this->height;
	Window::currentDPI = GetDpiForWindow(this->GetHWND()); 
	Logger::Warn(GetDpiForWindow(this->GetHWND()));

	if (this->isFullscreen) {
		this->ApplyFullscreenResolution(this->width, this->height);

		MONITORINFO mi{sizeof(mi)};
		if (GetMonitorInfo(MonitorFromWindow(this->hWnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
			SetWindowPos(this->hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
						 mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		} else {
			SetWindowPos(this->hWnd, HWND_TOP, 0, 0, this->width, this->height,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOMOVE);
		}
		UINT dpi = GetDpiForWindow(this->GetHWND()); 

		this->UpdateClientSize();
		if (this->resizeCallback) {
			this->resizeCallback(this->width, this->height, dpi);
		}
		return;
	}

	DWORD style = static_cast<DWORD>(GetWindowLongPtr(this->hWnd, GWL_STYLE));
	DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(this->hWnd, GWL_EXSTYLE));

	RECT desiredRect{0, 0, static_cast<LONG>(this->width), static_cast<LONG>(this->height)};
	if (AdjustWindowRectEx(&desiredRect, style, FALSE, exStyle)) {
		UINT windowWidth = static_cast<UINT>(desiredRect.right - desiredRect.left);
		UINT windowHeight = static_cast<UINT>(desiredRect.bottom - desiredRect.top);

		SetWindowPos(this->hWnd, HWND_TOP, 0, 0, windowWidth, windowHeight,
					 SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
	} else {
		SetWindowPos(this->hWnd, HWND_TOP, 0, 0, this->width, this->height,
					 SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
	}

	this->UpdateClientSize();
	UINT dpi = GetDpiForWindow(this->GetHWND()); 
	if (this->resizeCallback) {
		this->resizeCallback(this->width, this->height, dpi);
	}
}

void Window::ToggleFullscreen(bool fullscreen) {
	if (this->isFullscreen == fullscreen) {
		return;
	}

	this->isFullscreen = fullscreen;

	if (fullscreen) {
		GetWindowRect(this->hWnd, &this->windowedRect);
		SetWindowLongPtr(this->hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

		this->ApplyFullscreenResolution(this->width, this->height);

		MONITORINFO mi{sizeof(mi)};
		if (GetMonitorInfo(MonitorFromWindow(this->hWnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
			SetWindowPos(this->hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
						 mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}

		this->UpdateClientSize();
		UINT dpi = GetDpiForWindow(this->GetHWND()); 
		if (this->resizeCallback) {
			this->resizeCallback(this->width, this->height, dpi);
		}
	} else {
		if (this->hasOriginalDisplayMode) {
			ChangeDisplaySettings(&this->originalDisplayMode, 0);
		} else {
			ChangeDisplaySettings(nullptr, 0);
		}

		SetWindowLongPtr(this->hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		SetWindowPos(this->hWnd, HWND_TOP, this->windowedRect.left, this->windowedRect.top,
					 this->windowedRect.right - this->windowedRect.left,
					 this->windowedRect.bottom - this->windowedRect.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		this->UpdateClientSize();
		UINT dpi = GetDpiForWindow(this->GetHWND()); 
		if (this->resizeCallback) {
			this->resizeCallback(this->width, this->height, dpi);
		}
	}
}

void Window::SetResizeCallback(std::function<void(UINT, UINT, UINT)> callback) { this->resizeCallback = std::move(callback); }
