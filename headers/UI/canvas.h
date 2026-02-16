#pragma once

#include "UI/widget.h"

// std
#include <memory>
#include <vector>

namespace UI {

class Canvas {
public:
	Canvas() = default;

	// Size in screen space
	void SetSize(Vec2 size);
	Vec2 GetSize() const;

	// Root-level widgets
	void AddChild(const std::shared_ptr<Widget>& child);
	void RemoveChild(const std::shared_ptr<Widget>& child);
	void Clear();
	const std::vector<std::shared_ptr<Widget>>& GetChildren() const;

	void Update(float dt);
	void Draw();

	std::shared_ptr<Widget> HitTest(Vec2 point) const;

private:
	Vec2 size{};
	std::vector<std::shared_ptr<Widget>> children;

	std::shared_ptr<Widget> HitTestRecursive(const std::shared_ptr<Widget>& node, Vec2 point) const;
};

} // namespace UI