#include "core/game.h"
#include "core/imguiManager.h"
#include "scene/sceneManager.h"
#include "utilities/time.h"
#include "core/input/inputManager.h"
#include <memory>
#include "core/filepathHolder.h"

// Game Loop
void Game::Run(HINSTANCE hInstance, int nCmdShow)
{
    Window window(hInstance, nCmdShow, "Game Window");

    FilepathHolder::SetDirectiories();

    this->renderer.Init(window);
    AssetManager::GetInstance().SetDevicePointer(this->renderer.GetDevice());
    AssetManager::GetInstance().CreateDefaultAssets();
    this->renderer.SetAllDefaults();
    this->sceneManager = std::make_unique<SceneManager>(&renderer);

    this->imguiManager.InitalizeImgui(window.GetHWND(), this->renderer.GetDevice(), this->renderer.GetContext());
    this->imguiManager.SetResolutionChangeCallback([&](UINT width, UINT height)
                                                   { window.Resize(width, height); });
    this->imguiManager.SetFullscreenChangeCallback([&](bool fullscreen)
                                                   { window.ToggleFullscreen(fullscreen); });
    window.SetResizeCallback([&](UINT, UINT)
                             { this->renderer.Resize(window); });
    this->imguiManager.SetVSyncChangeCallback([&](bool enable)
                                              { this->renderer.ToggleVSync(enable); });
    this->imguiManager.SetSaveSceneChangeCallback([&](const std::string &filepath)
                                                  {
                                                    if (filepath.empty())
                                                    {
                                                        this->sceneManager->SaveSceneToCurrentFile();
                                                        return;
                                                    }
                                                    this->sceneManager->SaveSceneToFile(filepath); });
    this->imguiManager.SetSaveSceneAsChangeCallback([&](const std::string &filepath)
                                                    { this->sceneManager->SaveSceneToFile(filepath); });
    this->imguiManager.SetLoadSceneChangeCallback([&](const std::string &filepath)
                                                  { this->sceneManager->LoadSceneFromFile(filepath); });

    this->imguiManager.SetWireframeChangeCallback([&](bool enable) { this->renderer.ToggleWireframe(enable); });
	this->imguiManager.SetPauseExecutionChangeCallback([&](bool enable) { this->sceneManager->TogglePause(enable); });

    this->sceneManager->LoadScene(SceneManager::Scenes::DEMO);


   
    MSG msg = {};

    while (msg.message != WM_QUIT)
    {
        this->imguiManager.ImguiAtFrameStart();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Time::GetInstance().Tick();
        this->sceneManager->SceneTick();

        this->renderer.Render();
        this->imguiManager.ImguiAtFrameEnd();
        this->renderer.Present();
        
        InputManager::GetInstance().Reset();
    }
}

std::filesystem::path FilepathHolder::exeDirectory;
std::filesystem::path FilepathHolder::assetsDirectory;

bool ImguiManager::showObjectHierarchy = false;
