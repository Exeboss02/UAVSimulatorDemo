#include "game/hud.h"
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
		this->lubricantIcon = this->MakeIcon("HUD_Carbon_Icon", "assets/images/carbon_fiber.png", -padding,
											 startPos + IconOffset * 2, iconSize, UI::Anchor::TopRight);
		this->lubricantText =
			this->MakeText("HUD_Lubricant", "0", iconTextOffsetX, startPos + iconTextOffsetY + IconOffset * 2,
						   textWidth, UI::Anchor::TopRight);
	}
	// Circuit
	{
		this->lubricantIcon = this->MakeIcon("HUD_Circuit_Icon", "assets/images/circuit_board.png", -padding,
											 startPos + IconOffset * 3, iconSize, UI::Anchor::TopRight);
		this->lubricantText =
			this->MakeText("HUD_Lubricant", "0", iconTextOffsetX, startPos + iconTextOffsetY + IconOffset * 3,
						   textWidth, UI::Anchor::TopRight);
	}
	// Player health
	{
		this->playerHealthIcon = this->MakeIcon("HUD_Player_Health_Icon", "assets/images/health.png", padding, padding,
												iconSize, UI::Anchor::TopLeft);
		this->playerHealthText = this->MakeText("HUD_Player_Health", "0", -iconTextOffsetX, padding + iconTextOffsetY,
												textWidth, UI::Anchor::TopLeft);
	}

	{
		this->storyText = this->MakeText("HUD_Player_Story_Text", " ", 0, -20, 0, UI::Anchor::BottomCenter);
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
	}
}

void HUD::Update(const ResourceManager& resources, const Health& playerHealth) {
	this->SafeTextSet(this->titaniumText, std::to_string(resources.titanium.GetAmount()));
	this->SafeTextSet(this->lubricantText, std::to_string(resources.lubricant.GetAmount()));
	this->SafeTextSet(this->carbonFiberText, std::to_string(resources.carbonFiber.GetAmount()));
	this->SafeTextSet(this->circuitText, std::to_string(resources.circuit.GetAmount()));
	this->SafeTextSet(this->playerHealthText, std::to_string(playerHealth.Get()));
}

void HUD::OnDestroy() {
	if (this->canvasObj.expired()) return;

	auto canvasShared = this->canvasObj.lock();
	if (!canvasShared) return;

	canvasShared->Clear();
}

void HUD::SetStoryText(const std::string& text) { this->SafeTextSet(this->storyText, text); }

void HUD::SetStoryTextVisibility(bool visible) {
	if (auto storyText = this->storyText.lock()) {
		storyText->SetVisible(visible);
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
