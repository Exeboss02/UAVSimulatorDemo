#pragma once

#include "UI/widget.h"

namespace UI {

class ScrollableWidget : public Widget {
public:
	ScrollableWidget() = default;
	virtual ~ScrollableWidget() = default;

	void SetAutoScroll(bool enabled);
	bool IsAutoScrollEnabled() const;
	void SetScrollSpeed(float pixelsPerSecond);
	float GetScrollSpeed() const;
	void SetScrollLoop(bool enabled);
	bool IsScrollLoopEnabled() const;
	void SetScrollResetY(float y);
	float GetScrollResetY() const;
	void SetScrollTopLimitY(float y);
	float GetScrollTopLimitY() const;

	virtual void Update(float dt) override;
	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

protected:
	void ShowScrollControlsInHierarchy();

	bool autoScroll = false;
	float scrollSpeed = 0.0f;
	bool scrollLoop = false;
	float scrollResetY = -1.0f;
	float scrollTopLimitY = -200.0f;
};

} // namespace UI
