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
	} catch (const std::exception &e) {
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
		auto [ix, iy, tx, ty] = makeLeftAlignedPositions(5);
		this->playerHealthText = makeText("HUD_Player_Story_Text", "", tx, ty, textWidth, false);
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

void HUD::SafeTextSet(std::weak_ptr<UI::Text> textObjectWeak, const std::string& text) {
	if (textObjectWeak.expired()) return;
	auto textObject = textObjectWeak.lock();
	textObject->SetText(text);
}
