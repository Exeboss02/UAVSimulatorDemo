#pragma once

#include "UI/widget.h"
#include <functional>
#include <memory>

class ResourceManager;
class GameObjectFactory;

namespace UI {
class CanvasObject;
class Text;
class Image;
class InteractionPrompt;
class Button;
} // namespace UI

class Crosshair;
class Health;

class HUD {
public:
	explicit HUD(GameObjectFactory* factory);
	~HUD();

	// Create UI objects (call once after construction)
	void Start();

	// Update displayed values from the player's health and resource
	void Update(const ResourceManager& resources, const Health& playerHealth);

	// Clean up (queue deletes) if needed
	void OnDestroy();

	// Quit-to-menu prompt controls
	void ShowQuitToMenuPrompt();
	void HideQuitToMenuPrompt();
	bool IsQuitPromptVisible() const;

	// Callback invoked when the quit prompt is hidden (e.g. NO pressed)
	void SetOnQuitPromptHidden(std::function<void()> cb) { this->onQuitPromptHidden = std::move(cb); }
	void SetStoryText(const std::string& text);

	void SetStoryTextVisibility(bool visible);

private:
	std::weak_ptr<UI::Text> MakeText(const std::string& name, const std::string& text, float x, float y, float width,
									 UI::Anchor anchor);

	std::weak_ptr<UI::Image> MakeIcon(const std::string& name, const std::string& imagePath, float x, float y,
									  float size, UI::Anchor anchor);

	void SafeTextSet(std::weak_ptr<UI::Text> textObject, const std::string& text);

	GameObjectFactory* factory = nullptr;

	std::weak_ptr<UI::CanvasObject> canvasObj;

	std::weak_ptr<UI::Image> titaniumIcon;
	std::weak_ptr<UI::Text> titaniumText;

	std::weak_ptr<UI::Image> lubricantIcon;
	std::weak_ptr<UI::Text> lubricantText;

	std::weak_ptr<UI::Image> carbonFiberIcon;
	std::weak_ptr<UI::Text> carbonFiberText;

	std::weak_ptr<UI::Image> circuitIcon;
	std::weak_ptr<UI::Text> circuitText;

	std::weak_ptr<UI::Image> playerHealthIcon;
	std::weak_ptr<UI::Text> playerHealthText;

	std::weak_ptr<UI::Text> storyText;

	std::weak_ptr<Crosshair> crosshair;

	// Shared prompt created once and reused for interactions
	std::weak_ptr<UI::InteractionPrompt> interactionPrompt;

	// Quit modal widgets
	std::weak_ptr<UI::Text> quitPromptText;
	std::weak_ptr<UI::Button> quitYesButton;
	std::weak_ptr<UI::Button> quitNoButton;
	std::weak_ptr<UI::Image> quitBackground;
	bool quitPromptVisible = false;

	std::function<void()> onQuitPromptHidden;
};
