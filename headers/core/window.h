#pragma once

#include "core/input/inputManager.h"
#include <functional>

// std
#include <Windows.h>
#include <memory>
#include <string>
#include <windowsx.h>

class Window {
public:
	Window(const HINSTANCE instance, int nCmdShow, const std::string name = "Window", const UINT width = 1024,
		   const UINT height = 576);
	~Window();

	HWND GetHWND() const;
	UINT GetWidth() const;
	UINT GetHeight() const;
	bool IsFullscreen() const;
	void Show(int nCmdShow) const;

	void Show(int nCmdShow);
	void Resize(UINT width, UINT height);
	void ToggleFullscreen(bool fullscreen);

	void SetResizeCallback(std::function<void(UINT, UINT)> callback);

private:
	HWND hWnd;
	UINT width;
	UINT height;
	HINSTANCE instance;
	RECT windowedRect{};
	bool isFullscreen;

	DEVMODE originalDisplayMode{};
	bool hasOriginalDisplayMode;

	static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void UpdateClientSize();
	void ApplyFullscreenResolution(UINT width, UINT height);

	std::function<void(UINT, UINT)> resizeCallback;
};