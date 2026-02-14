#pragma once

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// std
#include <Windows.h>
#include <d3d11.h>
#include <functional>
#include <string>

class ImguiManager {
public:
	ImguiManager() = default;
	ImguiManager(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* immediateContext);
	~ImguiManager();

	void InitalizeImgui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* immediateContext);

	void ImguiAtFrameStart();
	void ImguiAtFrameEnd();

	void SetResolutionChangeCallback(std::function<void(UINT, UINT)> callback);
	void SetFullscreenChangeCallback(std::function<void(bool)> callback);
	void SetVSyncChangeCallback(std::function<void(bool)> callback);
	void SetWireframeChangeCallback(std::function<void(bool)> callback);
	void SetPauseExecutionChangeCallback(std::function<void(bool)> callback);
	void SetSaveSceneChangeCallback(std::function<void(const std::string&)> callback);
	void SetSaveSceneAsChangeCallback(std::function<void(const std::string&)> callback);
	void SetLoadSceneChangeCallback(std::function<void(const std::string&)> callback);

	// Edit
	static bool showObjectHierarchy;

private:
	void ConsoleImGui();
	void WindowOptionsImGui();
	void MainMenuImGui();
	float GetAverageFrameRate();

	// Options
	bool showOptionsWindow = false;

	// Window
	bool isFullscreen = false;
	bool isVSync = false;

	// Debug
	bool showConsoleWindow = false;
	bool showWireframe = false;
	bool pauseExecution = false;

	// Edit
	bool showTransformWindow = false;
	bool showSoundWindow = false;
	bool showMusicWindow = false;

	std::function<void(UINT, UINT)> resolutionChangeCallback;
	std::function<void(bool)> fullscreenChangeCallback;
	std::function<void(bool)> vSyncChangeCallback;
	std::function<void(bool)> wireframeChangeCallback;
	std::function<void(bool)> pauseChangeCallback;
	std::function<void(const std::string&)> saveSceneChangeCallback;
	std::function<void(const std::string&)> saveSceneAsChangeCallback;
	std::function<void(const std::string&)> loadSceneChangeCallback;
	std::string currentScenePath;
};