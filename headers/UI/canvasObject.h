#pragma once

// std
#include <memory>

#include "gameObjects/gameObject.h"

namespace UI {

class Canvas;
class Widget;
struct Vec2;

class CanvasObject : public GameObject {
public:
	CanvasObject() = default;
	~CanvasObject() override = default;
	explicit CanvasObject(const std::shared_ptr<Canvas>& canvas);

	void SetCanvas(const std::shared_ptr<Canvas>& canvas);
	std::shared_ptr<Canvas> GetCanvas() const;
	bool HasCanvas() const;

	// Proxy methods to avoid duplicating canvas functionality
	void AddChild(const std::shared_ptr<Widget>& child);
	void RemoveChild(const std::shared_ptr<Widget>& child);
	void Clear();

	void Update(float dt);
	void Draw();

	std::shared_ptr<Widget> HitTest(Vec2 point) const;

	// GameObject lifecycle
	void Start() override;
	void Tick() override;
	void OnDestroy() override;

	// Show canvas-specific options in the inspector/hierarchy
	void ShowInHierarchy() override;

	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

private:
	std::shared_ptr<Canvas> canvas; // owned Canvas for this CanvasObject
};

} // namespace UI
