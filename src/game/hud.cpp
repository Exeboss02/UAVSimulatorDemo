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

	auto makeText = [&](const std::string& name, const std::string& text, float x, float y, float width,
						bool rightAligned) {
		auto textWeak = this->factory->CreateGameObjectOfType<UI::Text>();
		if (textWeak.expired()) return std::weak_ptr<UI::Text>();

		auto textShared = textWeak.lock();
		textShared->SetName(name);
		textShared->SetText(text);
		textShared->SetPosition(UI::Vec2{x, y});
		textShared->SetSize(UI::Vec2{width, 32.0f});
		textShared->SetFontSize(24.0f);

		// Alignment: use right-aligned behavior only when requested
		textShared->SetRightAligned(rightAligned);
		if (rightAligned) {
			textShared->SetMaxWidth(width);
		} else {
			textShared->SetMaxWidth(0.0f);
		}

		std::weak_ptr<GameObject> me = canvasShared->GetPtr();
		textShared->SetParent(me);
		canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(textShared));
		RenderQueue::AddUIWidget(textShared);
		return std::weak_ptr<UI::Text>(textShared);
	};

	auto makeIcon = [&](const std::string& name, const std::string& imagePath, float x, float y, float size) {
		auto iconWeak = this->factory->CreateGameObjectOfType<UI::Image>();
		if (iconWeak.expired()) return std::weak_ptr<UI::Image>();

		auto iconShared = iconWeak.lock();
		iconShared->SetName(name);
		iconShared->SetImage(imagePath);
		iconShared->SetPosition(UI::Vec2{x, y});
		iconShared->SetSize(UI::Vec2{size, size});

		std::weak_ptr<GameObject> me = canvasShared->GetPtr();
		iconShared->SetParent(me);
		canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(iconShared));
		RenderQueue::AddUIWidget(iconShared);
		return std::weak_ptr<UI::Image>(iconShared);
	};

	const float iconSize = 96.0f;
	const float lineHeight = iconSize - 20.0f;
	const float padding = 10.0f;
	const float textWidth = 48.0f;

	float canvasWidth = 1920.0f;
	float canvasHeight = 1080.0f;
	try {
		if (auto canvasPtr = canvasShared->GetCanvas()) {
			auto sz = canvasPtr->GetSize();
			if (sz.x > 0.0f) canvasWidth = sz.x;
			if (sz.y > 0.0f) canvasHeight = sz.y;
		}
	} catch (const std::exception& e) {
		Logger::Error("HUD::Start canvas size query failed: ", e.what());
	} catch (...) {
		Logger::Error("HUD::Start canvas size query failed (unknown exception)");
	}

	auto makeRightAlignedPositions = [&](int row) {
		float iconX = canvasWidth - padding - iconSize;
		float iconY = padding + row * lineHeight;
		float textX = iconX - 8.0f - textWidth;
		// Vertically center the text next to the (now larger) icon
		float textY = iconY + (iconSize - 32.0f) * 0.5f;
		return std::tuple<float, float, float, float>(iconX, iconY, textX, textY);
	};

	auto makeLeftAlignedPositions = [&](int row) {
		float iconX = padding;
		float iconY = padding + row * lineHeight;
		float textX = iconX + iconSize + 8.0f;
		float textY = iconY + (iconSize - 32.0f) * 0.5f;
		return std::tuple<float, float, float, float>(iconX, iconY, textX, textY);
	};

	// Titanium
	{
		auto [ix, iy, tx, ty] = makeRightAlignedPositions(0);
		this->titaniumIcon = makeIcon("HUD_Titanium_Icon", "assets/images/metal.png", ix, iy, iconSize);
		this->titaniumText = makeText("HUD_Titanium", "0", tx, ty, textWidth, true);
	}
	// Lubricant
	{
		auto [ix, iy, tx, ty] = makeRightAlignedPositions(1);
		this->lubricantIcon = makeIcon("HUD_Lubricant_Icon", "assets/images/lubricant.png", ix, iy, iconSize);
		this->lubricantText = makeText("HUD_Lubricant", "0", tx, ty, textWidth, true);
	}
	// Carbon
	{
		auto [ix, iy, tx, ty] = makeRightAlignedPositions(2);
		this->carbonFiberIcon = makeIcon("HUD_Carbon_Icon", "assets/images/carbon_fiber.png", ix, iy, iconSize);
		this->carbonFiberText = makeText("HUD_Carbon", "0", tx, ty, textWidth, true);
	}
	// Circuit
	{
		auto [ix, iy, tx, ty] = makeRightAlignedPositions(3);
		this->circuitIcon = makeIcon("HUD_Circuit_Icon", "assets/images/circuit_board.png", ix, iy, iconSize);
		this->circuitText = makeText("HUD_Circuit", "0", tx, ty, textWidth, true);
	}
	// Player health
	{
		auto [ix, iy, tx, ty] = makeLeftAlignedPositions(0);
		this->playerHealthIcon = makeIcon("HUD_Player_Health_Icon", "assets/images/health.png", ix, iy, iconSize);
		this->playerHealthText = makeText("HUD_Player_Health", "0", tx, ty, textWidth, false);
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
					bg->SetSize(UI::Vec2{canvasWidth, canvasHeight});
					bg->SetPosition(UI::Vec2{0.0f, 0.0f});
					bg->SetTint(DirectX::XMFLOAT4{0.0f, 0.0f, 0.0f, 0.5f});
					bg->SetVisible(false);
					// Ensure background is below the prompt buttons so they remain clickable
					bg->SetZIndex(0);
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
				// Size and position centered
				float w = canvasWidth;
				float h = canvasHeight;
				float tw = 600.0f;
				float th = 60.0f;
				quitText->SetSize(UI::Vec2{tw, th});
				quitText->SetPosition(UI::Vec2{((w - tw) + 120) * 0.5f, (h - th) * 0.5f - 350.0f});
				quitText->SetVisible(false);
				// Prompt text should be above the background but below buttons
				quitText->SetZIndex(1);
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
				yesBtn->SetPosition(UI::Vec2{(canvasWidth - 200.0f) * 0.5f - 120.0f, (canvasHeight) * 0.5f - 200.0f});
				yesBtn->SetVisible(false);
				// Buttons must be top-most so they receive input events
				yesBtn->SetZIndex(2);
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
				noBtn->SetSize(UI::Vec2{200.0f, 50.0f});
				noBtn->SetPosition(UI::Vec2{(canvasWidth - 200.0f) * 0.5f + 120.0f, (canvasHeight) * 0.5f - 200.0f});
				noBtn->SetVisible(false);
				// Buttons must be top-most so they receive input events
				noBtn->SetZIndex(2);
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
	auto safeSet = [](std::weak_ptr<UI::Text> w, const std::string& s) {
		if (w.expired()) return;
		auto p = w.lock();
		if (!p) return;
		p->SetText(s);
	};

	safeSet(this->titaniumText, std::to_string(resources.titanium.GetAmount()));
	safeSet(this->lubricantText, std::to_string(resources.lubricant.GetAmount()));
	safeSet(this->carbonFiberText, std::to_string(resources.carbonFiber.GetAmount()));
	safeSet(this->circuitText, std::to_string(resources.circuit.GetAmount()));
	safeSet(this->playerHealthText, std::to_string(playerHealth.Get()));
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
