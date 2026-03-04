#include "game/hud.h"
#include "UI/button.h"
#include "UI/canvas.h"
#include "UI/canvasObject.h"
#include "UI/image.h"
#include "UI/interactionPrompt.h"
#include "UI/text.h"
#include "game/crosshair.h"
#include "game/health.h"
#include "game/resourceManager.h"
#include "gameObjects/gameObject.h"
#include "gameObjects/gameObjectFactory.h"
#include "rendering/renderQueue.h"
#include "scene/sceneManager.h"

#include <DirectXMath.h>

// std
#include <memory>
#include <string>

HUD::HUD(GameObjectFactory* factory) : factory(factory) {}

HUD::~HUD() {}

void HUD::Start() {
	if (!this->factory) return;

	auto canvasWeak = this->factory->CreateGameObjectOfType<UI::CanvasObject>();
	if (canvasWeak.expired()) return;

	auto canvasShared = canvasWeak.lock();
	if (!canvasShared) return;

	canvasShared->SetName("PlayerHUD_Canvas");
	this->canvasObj = canvasShared;

	const float iconSize = 96.0f;
	const float lineHeight = iconSize - 20.0f;
	const float padding = 10.0f;
	const float textWidth = 48.0f;

	float startPos = 20;
	float IconOffset = iconSize + padding;
	float iconTextOffsetY = 40;
	float iconTextOffsetX = -iconSize - padding;

	UI::Vec2 canvasSize = {static_cast<float>(Window::GetCurrentWidth()),
						   static_cast<float>(Window::GetCurrentHeight())};

	// Titanium
	{
		this->titaniumIcon = this->MakeIcon("HUD_Titanium_Icon", "assets/images/metal.png", -padding, startPos,
											iconSize, UI::Anchor::TopRight);
		this->titaniumText = this->MakeText("HUD_Titanium", "0", iconTextOffsetX, startPos + iconTextOffsetY, textWidth,
											UI::Anchor::TopRight);
	}
	// Lubricant
	{
		this->lubricantIcon = this->MakeIcon("HUD_Lubricant_Icon", "assets/images/lubricant.png", -padding,
											 startPos + IconOffset * 1, iconSize, UI::Anchor::TopRight);
		this->lubricantText =
			this->MakeText("HUD_Lubricant", "0", iconTextOffsetX, startPos + iconTextOffsetY + IconOffset * 1,
						   textWidth, UI::Anchor::TopRight);
	}
	// Carbon
	{
		this->carbonFiberIcon = this->MakeIcon("HUD_Carbon_Icon", "assets/images/carbon_fiber.png", -padding,
											   startPos + IconOffset * 2, iconSize, UI::Anchor::TopRight);
		this->carbonFiberText =
			this->MakeText("HUD_Carbon", "0", iconTextOffsetX, startPos + iconTextOffsetY + IconOffset * 2, textWidth,
						   UI::Anchor::TopRight);
	}
	// Circuit
	{
		this->circuitIcon = this->MakeIcon("HUD_Circuit_Icon", "assets/images/circuit_board.png", -padding,
										   startPos + IconOffset * 3, iconSize, UI::Anchor::TopRight);
		this->circuitText =
			this->MakeText("HUD_Circuit", "0", iconTextOffsetX, startPos + iconTextOffsetY + IconOffset * 3, textWidth,
						   UI::Anchor::TopRight);
	}
	// Player health
	{
		this->playerHealthIcon = this->MakeIcon("HUD_Player_Health_Icon", "assets/images/health.png", padding, padding,
												iconSize, UI::Anchor::TopLeft);
		this->playerHealthText = this->MakeText("HUD_Player_Health", "0", -iconTextOffsetX, padding + iconTextOffsetY,
												textWidth, UI::Anchor::TopLeft);
	}
	// Core Health
	{
		this->coreHealthText =
			this->MakeText("HUD_Core_Health", "Core Health: 100", 0, 40, textWidth, UI::Anchor::TopCenter);
	}
	// Round indicator
	{
		this->roundIndicator = this->MakeText("HUD_Round_Indicator", " ", 20, 120, textWidth, UI::Anchor::TopLeft);
		this->roundIndicator.lock()->SetFontSize(18);
	}

	// Objective Text
	{
		this->objective = this->MakeText("HUD_Objective", "---TEMP---", 0, 100, textWidth, UI::Anchor::TopCenter);
		this->objective.lock()->SetFontSize(22);
		this->objective.lock()->SetVisible(false);
		this->objective.lock()->SetColor({1, 1, 0, 1});
	}

	// Blood overlay (visible on low health)
	{
		auto bloodWeak = this->factory->CreateGameObjectOfType<UI::Image>();
		if (!bloodWeak.expired()) {
			auto blood = bloodWeak.lock();
			blood->SetName("HUD_Blood_Overlay");
			blood->SetImage("assets/images/blood.png");
			blood->SetSize(canvasSize);
			blood->SetPosition(UI::Vec2{0.0f, 0.0f});
			blood->SetAnchor(UI::Anchor::TopLeft);
			blood->SetVisible(false);
			blood->SetZIndex(-1);
			std::weak_ptr<GameObject> me = canvasShared->GetPtr();
			blood->SetParent(me);
			canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(blood));
			RenderQueue::AddUIWidget(blood);
			this->bloodOverlay = blood;
		}
	}

	{
		this->storyText = this->MakeText("HUD_Player_Story_Text", " ", 0, -60, 0, UI::Anchor::BottomCenter);
	}

	{
		auto crossWeak = this->factory->CreateGameObjectOfType<Crosshair>();
		if (!crossWeak.expired()) {
			auto crossShared = crossWeak.lock();
			if (crossShared) {
				crossShared->SetName("HUD_Crosshair");
				crossShared->SetCrosshairImage("assets/images/crosshair.png");
				crossShared->SetCrosshairSize(48.0f, 48.0f);

				std::weak_ptr<GameObject> me = canvasShared->GetPtr();
				crossShared->SetParent(me);
				canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(crossShared));
				RenderQueue::AddUIWidget(crossShared);
				this->crosshair = crossShared;
			}
		}

		// Create a single shared interaction prompt owned by HUD
		{
			auto promptWeak = this->factory->CreateGameObjectOfType<UI::InteractionPrompt>();
			if (!promptWeak.expired()) {
				auto promptShared = promptWeak.lock();
				if (promptShared) {
					promptShared->SetName("HUD_InteractionPrompt");
					this->interactionPrompt = promptShared;
				}
			}
		}

		// Create quit-to-menu prompt (hidden by default)
		{
			// Semi-opaque background covering the whole canvas
			{
				auto bgWeak = this->factory->CreateGameObjectOfType<UI::Image>();
				if (!bgWeak.expired()) {
					auto bg = bgWeak.lock();
					bg->SetName("HUD_Quit_Background");
					bg->SetSize(canvasSize);
					bg->SetPosition(UI::Vec2{0.0f, 0.0f});
					bg->SetTint(DirectX::XMFLOAT4{0.0f, 0.0f, 0.0f, 0.5f});
					bg->SetVisible(false);
					// Keep menu above all HUD widgets while preserving internal menu layering
					bg->SetZIndex(100);
					std::weak_ptr<GameObject> me = canvasShared->GetPtr();
					bg->SetParent(me);
					canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(bg));
					RenderQueue::AddUIWidget(bg);
					this->quitBackground = bg;
				}
			}

			// Centered text
			auto quitTextWeak = this->factory->CreateGameObjectOfType<UI::Text>();
			if (!quitTextWeak.expired()) {
				auto quitText = quitTextWeak.lock();
				quitText->SetName("HUD_QuitPrompt_Text");
				quitText->SetText("Do you want to quit to menu?");
				quitText->SetFontSize(36.0f);

				float tw = 400.0f;
				float th = 60.0f;
				quitText->SetSize(UI::Vec2{tw, th});
				quitText->SetAnchor(UI::Anchor::TopCenter);
				quitText->SetPosition(UI::Vec2{0.0f, 100.0f});
				quitText->SetVisible(false);
				// Prompt text should be above background and below buttons
				quitText->SetZIndex(101);
				std::weak_ptr<GameObject> me = canvasShared->GetPtr();
				quitText->SetParent(me);
				canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(quitText));
				RenderQueue::AddUIWidget(quitText);
				this->quitPromptText = quitText;
			}

			// Yes button
			auto yesWeak = this->factory->CreateGameObjectOfType<UI::Button>();
			if (!yesWeak.expired()) {
				auto yesBtn = yesWeak.lock();
				yesBtn->SetName("HUD_Quit_Yes");
				yesBtn->SetLabel("YES");
				yesBtn->SetSize(UI::Vec2{200.0f, 50.0f});
				yesBtn->SetAnchor(UI::Anchor::TopCenter);
				yesBtn->SetPosition(UI::Vec2{-200.0f, 200.0f});
				yesBtn->SetVisible(false);
				// Buttons are top-most among menu widgets
				yesBtn->SetZIndex(102);
				std::weak_ptr<GameObject> me = canvasShared->GetPtr();
				yesBtn->SetParent(me);
				canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(yesBtn));
				RenderQueue::AddUIWidget(yesBtn);
				this->quitYesButton = yesBtn;
				// On click -> schedule main menu load at end of tick (avoid deleting current objects during update)
				yesBtn->SetOnClick([]() {
					if (auto sm = SceneManager::GetActive()) {
						auto path = (FilepathHolder::GetAssetsDirectory() / "scenes" / "MainMenu.scene").string();
						sm->QueueLoadSceneFile(path);
					}
				});
			}

			// No button
			auto noWeak = this->factory->CreateGameObjectOfType<UI::Button>();
			if (!noWeak.expired()) {
				auto noBtn = noWeak.lock();
				noBtn->SetName("HUD_Quit_No");
				noBtn->SetLabel("NO");
				noBtn->SetHorizontalAlign(UI::Button::HorizontalAlign::CENTER);
				noBtn->SetSize(UI::Vec2{200.0f, 50.0f});
				noBtn->SetAnchor(UI::Anchor::TopCenter);
				noBtn->SetPosition(UI::Vec2{200.0f, 200.0f});
				noBtn->SetVisible(false);
				// Buttons are top-most among menu widgets
				noBtn->SetZIndex(102);
				std::weak_ptr<GameObject> me = canvasShared->GetPtr();
				noBtn->SetParent(me);
				canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(noBtn));
				RenderQueue::AddUIWidget(noBtn);
				this->quitNoButton = noBtn;
				// On click -> hide prompt
				noBtn->SetOnClick([this]() { this->HideQuitToMenuPrompt(); });
			}
		}
	}
}

void HUD::Update(const ResourceManager& resources, const Health& playerHealth) {
	this->SafeTextSet(this->titaniumText, std::to_string(resources.titanium.GetAmount()));
	this->SafeTextSet(this->lubricantText, std::to_string(resources.lubricant.GetAmount()));
	this->SafeTextSet(this->carbonFiberText, std::to_string(resources.carbonFiber.GetAmount()));
	this->SafeTextSet(this->circuitText, std::to_string(resources.circuit.GetAmount()));
	this->SafeTextSet(this->playerHealthText, std::to_string(playerHealth.Get()));

	if (auto blood = this->bloodOverlay.lock()) {
		if (auto canvasObj = this->canvasObj.lock()) {
			if (auto canvas = canvasObj->GetCanvas()) {
				blood->SetSize(canvas->GetSize());
			}
		}
		blood->SetVisible(playerHealth.Get() <= 20);
	}
}

void HUD::OnDestroy() {
	if (this->canvasObj.expired()) return;

	auto canvasShared = this->canvasObj.lock();
	if (!canvasShared) return;

	canvasShared->Clear();
}

void HUD::ShowQuitToMenuPrompt() {
	// Avoid re-showing when already visible
	if (this->quitPromptVisible) return;

	// Show widgets if created
	if (!this->quitBackground.expired()) {
		auto bg = this->quitBackground.lock();
		if (bg) bg->SetVisible(true);
	}
	if (!this->quitPromptText.expired()) {
		auto t = this->quitPromptText.lock();
		if (t) t->SetVisible(true);
	}
	if (!this->quitYesButton.expired()) {
		auto b = this->quitYesButton.lock();
		if (b) b->SetVisible(true);
	}
	if (!this->quitNoButton.expired()) {
		auto b = this->quitNoButton.lock();
		if (b) b->SetVisible(true);
	}
	this->quitPromptVisible = true;
}

void HUD::HideQuitToMenuPrompt() {
	if (!this->quitBackground.expired()) {
		auto bg = this->quitBackground.lock();
		if (bg) bg->SetVisible(false);
	}
	if (!this->quitPromptText.expired()) {
		auto t = this->quitPromptText.lock();
		if (t) t->SetVisible(false);
	}
	if (!this->quitYesButton.expired()) {
		auto b = this->quitYesButton.lock();
		if (b) b->SetVisible(false);
	}
	if (!this->quitNoButton.expired()) {
		auto b = this->quitNoButton.lock();
		if (b) b->SetVisible(false);
	}
	this->quitPromptVisible = false;

	// Notify listeners that the prompt was hidden (e.g. restore player input)
	try {
		if (this->onQuitPromptHidden) this->onQuitPromptHidden();
	} catch (...) {
		// Swallow exceptions from callbacks to avoid crashing the HUD hide path
	}
}

bool HUD::IsQuitPromptVisible() const { return this->quitPromptVisible; }
void HUD::SetStoryText(const std::string& text) { this->SafeTextSet(this->storyText, text); }

void HUD::SetStoryTextVisibility(bool visible) {
	if (auto storyText = this->storyText.lock()) {
		storyText->SetVisible(visible);
	}
}

void HUD::SetCoreHealthText(int health) {
	this->SafeTextSet(this->coreHealthText, std::format("Core Health: {}", health));
}

void HUD::SetRoundIndicator(size_t roundsLeft, float timeUntilNextRound, bool showTime) {

	std::string text = std::format("Pirate ships left: {}", roundsLeft) +
					   (showTime ? std::format("\nTime until pirate breach: {}", (int) timeUntilNextRound + 1) : "");

	this->SafeTextSet(this->roundIndicator, text);
}

void HUD::SetObjective(const std::string& objective) {
	this->SafeTextSet(this->objective, std::format("Objective: {}", objective));
}

void HUD::SetObjectiveVisible(bool visible) {
	if (auto text = this->objective.lock()) {
		text->SetVisible(visible);
	}
}

std::weak_ptr<UI::Text> HUD::MakeText(const std::string& name, const std::string& text, float x, float y, float width,
									  UI::Anchor anchor) {
	if (this->canvasObj.expired()) {
		std::string error = "MakeText was called before canvas existed";
		Logger::Error(error);
		throw std::runtime_error(error);
	}
	auto textWeak = this->factory->CreateGameObjectOfType<UI::Text>();
	if (textWeak.expired()) return std::weak_ptr<UI::Text>();

	auto textShared = textWeak.lock();
	textShared->SetName(name);
	textShared->SetText(text);
	textShared->SetPosition(UI::Vec2{x, y});
	textShared->SetSize(UI::Vec2{width, 32.0f});
	textShared->SetFontSize(24.0f);
	textShared->SetFont("Lucida Console");

	// Alignment: use right-aligned behavior only when requested
	textShared->SetAnchor(anchor);

	auto canvasShared = this->canvasObj.lock();

	std::weak_ptr<GameObject> me = canvasShared->GetPtr();
	textShared->SetParent(me);
	canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(textShared));
	RenderQueue::AddUIWidget(textShared);
	return std::weak_ptr<UI::Text>(textShared);
}

std::weak_ptr<UI::Image> HUD::MakeIcon(const std::string& name, const std::string& imagePath, float x, float y,
									   float size, UI::Anchor anchor) {
	if (this->canvasObj.expired()) {
		std::string error = "MakeText was called before canvas existed";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	auto canvasShared = this->canvasObj.lock();

	auto textWeak = this->factory->CreateGameObjectOfType<UI::Text>();
	if (textWeak.expired()) return std::weak_ptr<UI::Image>();

	auto iconShared = this->factory->CreateGameObjectOfType<UI::Image>().lock();

	iconShared->SetName(name);
	iconShared->SetImage(imagePath);
	iconShared->SetPosition(UI::Vec2{x, y});
	iconShared->SetSize(UI::Vec2{size, size});
	iconShared->SetAnchor(anchor);

	std::weak_ptr<GameObject> me = canvasShared->GetPtr();
	iconShared->SetParent(me);
	canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(iconShared));
	RenderQueue::AddUIWidget(iconShared);
	return std::weak_ptr<UI::Image>(iconShared);
}

void HUD::SafeTextSet(std::weak_ptr<UI::Text> textObjectWeak, const std::string& text) {
	if (textObjectWeak.expired()) return;
	auto textObject = textObjectWeak.lock();
	textObject->SetText(text);
}
