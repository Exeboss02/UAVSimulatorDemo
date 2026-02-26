#include "UI/interactionPrompt.h"
#include "UI/text.h"
#include "UI/textRenderer.h"
#include "core/window.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/gameObject.h"
#include "gameObjects/gameObjectFactory.h"
#include "rendering/renderQueue.h"
#include <DirectXMath.h>

void UI::InteractionPrompt::Start() {
	if (!this->factory) return;

	auto textWeak = this->factory->CreateGameObjectOfType<UI::Text>();
	if (textWeak.expired()) return;

	auto textShared = textWeak.lock();
	textShared->SetName("InteractionPrompt_Text");
	textShared->SetText("");
	textShared->SetFontSize(this->fontSize);
	textShared->SetVisible(false);
	textShared->SetSize(UI::Vec2{200.0f, 28.0f});

	std::weak_ptr<GameObject> me = this->GetPtr();
	textShared->SetParent(me);
	this->AddChild(std::static_pointer_cast<UI::Widget>(textShared));
	RenderQueue::AddUIWidget(textShared);

	this->textWidget = textShared;
}

void UI::InteractionPrompt::Show(const std::string& text, DirectX::XMVECTOR worldPos) {
	if (this->textWidget.expired()) return;
	auto textShared = this->textWidget.lock();
	if (!textShared) return;

	try {
		textShared->SetText(text);
		textShared->SetFontSize(this->fontSize);

		float w = static_cast<float>(Window::GetCurrentWidth());
		float h = static_cast<float>(Window::GetCurrentHeight());

		UI::Vec2 size = textShared->GetSize();
		if (size.x <= 0.0f) size.x = 200.0f;

		float sy = (h - size.y) * 0.5f + this->offsetY;

		float px = ((w - size.x) * 0.5f) + this->offsetX;
		textShared->SetSize(UI::Vec2{size.x, size.y});
		textShared->SetPosition(UI::Vec2{px, sy});
		textShared->SetVisible(true);

	} catch (const std::exception& e) {
		Logger::Error("UI::InteractionPrompt::Show exception: ", e.what());
	} catch (...) {
		Logger::Error("UI::InteractionPrompt::Show unknown exception");
	}
}

void UI::InteractionPrompt::Hide() {
	if (this->textWidget.expired()) return;
	auto textShared = this->textWidget.lock();
	if (!textShared) return;
	textShared->SetVisible(false);
}
