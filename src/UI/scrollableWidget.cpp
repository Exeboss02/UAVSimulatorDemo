#include "UI/scrollableWidget.h"

using namespace UI;

void ScrollableWidget::SetAutoScroll(bool enabled) { this->autoScroll = enabled; }
bool ScrollableWidget::IsAutoScrollEnabled() const { return this->autoScroll; }

void ScrollableWidget::SetScrollSpeed(float pixelsPerSecond) { this->scrollSpeed = pixelsPerSecond; }
float ScrollableWidget::GetScrollSpeed() const { return this->scrollSpeed; }

void ScrollableWidget::SetScrollLoop(bool enabled) { this->scrollLoop = enabled; }
bool ScrollableWidget::IsScrollLoopEnabled() const { return this->scrollLoop; }

void ScrollableWidget::SetScrollResetY(float y) { this->scrollResetY = y; }
float ScrollableWidget::GetScrollResetY() const { return this->scrollResetY; }

void ScrollableWidget::SetScrollTopLimitY(float y) { this->scrollTopLimitY = y; }
float ScrollableWidget::GetScrollTopLimitY() const { return this->scrollTopLimitY; }

void ScrollableWidget::Update(float dt) {
	if (this->autoScroll && this->enabled) {
		Vec2 localPos = this->position;
		localPos.y += this->scrollSpeed * dt;
		this->SetPosition(localPos);

		if (this->scrollLoop && this->scrollSpeed < 0.0f) {
			const float currentBottom = this->GetPosition().y + this->GetSize().y;
			if (currentBottom < this->scrollTopLimitY) {
				const float fallbackResetY = this->canvasSize.y + 20.0f;
				localPos.y = (this->scrollResetY >= 0.0f) ? this->scrollResetY : fallbackResetY;
				this->SetPosition(localPos);
			}
		}
	}

	Widget::Update(dt);
}

void ScrollableWidget::LoadFromJson(const nlohmann::json& data) {
	this->Widget::LoadFromJson(data);

	if (data.contains("autoScroll")) this->SetAutoScroll(data.at("autoScroll").get<bool>());
	if (data.contains("scrollSpeed")) this->SetScrollSpeed(data.at("scrollSpeed").get<float>());
	if (data.contains("scrollLoop")) this->SetScrollLoop(data.at("scrollLoop").get<bool>());
	if (data.contains("scrollResetY")) this->SetScrollResetY(data.at("scrollResetY").get<float>());
	if (data.contains("scrollTopLimitY")) this->SetScrollTopLimitY(data.at("scrollTopLimitY").get<float>());
}

void ScrollableWidget::SaveToJson(nlohmann::json& data) {
	this->Widget::SaveToJson(data);
	data["autoScroll"] = this->IsAutoScrollEnabled();
	if (this->IsAutoScrollEnabled()) {
		data["scrollSpeed"] = this->GetScrollSpeed();
		data["scrollLoop"] = this->IsScrollLoopEnabled();
		if (this->IsScrollLoopEnabled()) {
			data["scrollResetY"] = this->GetScrollResetY();
			data["scrollTopLimitY"] = this->GetScrollTopLimitY();
		}
	}
}

void ScrollableWidget::ShowScrollControlsInHierarchy() {
	ImGui::Separator();
	ImGui::Text("Scroll settings:");
	ImGui::Checkbox("Auto Scroll", &this->autoScroll);
	if (this->autoScroll) {
		ImGui::DragFloat("Scroll Speed", &this->scrollSpeed, 1.0f, -2000.0f, 2000.0f, "%.1f px/s");
		ImGui::Checkbox("Loop Scroll", &this->scrollLoop);
		if (this->scrollLoop) {
			ImGui::DragFloat("Scroll Top Limit Y", &this->scrollTopLimitY, 1.0f, -5000.0f, 5000.0f);
			ImGui::DragFloat("Scroll Reset Y", &this->scrollResetY, 1.0f, -5000.0f, 5000.0f);
		}
	}
}
