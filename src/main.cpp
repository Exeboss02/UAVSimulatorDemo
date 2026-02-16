#include "Windows.h"
#include "core/game.h"
#include "gameObjects/objectLoader.h"
#include "test.h"
#include "utilities/logger.h"
#include <filesystem>
#include <iostream>

// Main code
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
					  _In_ int nCmdShow) {
	// Normal batchfile run "C:\\Users\\Simon Nilsson\\source\\repos\\lillaspel\\build\\Debug"
	//					          C:\Users\Simon Nilsson\source\repos\LillaSpel\out\build\x64-debug\Debug\lillaspel.exe
	// Visual studio run "C:\\Users\\Simon Nilsson\\source\\repos\\LillaSpel\\out\\build\\x64-debug"
	// Some memory leak flags here
	//_CrtDumpMemoryLeaks();
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	std::unique_ptr<Game> game = std::unique_ptr<Game>(new Game());

	Logger::Log("Starting game...");
	try {
		game->Run(hInstance, nCmdShow);
	} catch (const std::exception& e) {
		Logger::Error(e.what());
	}
	int x = 5;
	Logger::Log("Exiting game...", x, game.get());
}
