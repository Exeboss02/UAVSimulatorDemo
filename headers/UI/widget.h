#pragma once

#include "gameObjects/meshObjData.h"
#include "gameObjects/meshObject.h"

// std
#include <memory>
#include <vector>

namespace UI {

struct Vec2 {
	float x = 0.0f;
	float y = 0.0f;
};

class Widget : public MeshObject {
public:
	Widget() = default;
	explicit Widget(MeshObjData& mesh);

	// Core properties
	void SetPosition(Vec2 pos);
	void SetSize(Vec2 size);
	Vec2 GetPosition() const;
	Vec2 GetSize() const;

	void SetVisible(bool visible);
	bool IsVisible() const;

	void SetEnabled(bool enabled);
	bool isEnabled() const;

	// Hierarchy
	virtual void Update(float dt);
	virtual void Draw();
	virtual bool HitTest(Vec2 point) const;

	void SetWidgetMesh(MeshObjData& mesh);

protected:
	Vec2 position{};
	Vec2 size{};

	bool visible = true;
	bool enabled = true;

	Widget* parent = nullptr;
	std::vector<std::shared_ptr<Widget>> children;
};

} // namespace UI