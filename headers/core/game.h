#pragma once

#include "core/imguiManager.h"
#include "core/window.h"
#include "rendering/renderer.h"
#include "scene/sceneManager.h"
#include "utilities/logger.h"

class Game {
public:
	Game() = default;
	~Game() = default;
	void Run(HINSTANCE hInstance, int nCmdShow);

	// audio manager has been moved to scene manager for better access

private:
	ImguiManager imguiManager;
	Renderer renderer;
	std::unique_ptr<SceneManager> sceneManager;
};