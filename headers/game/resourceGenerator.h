#pragma once
#include "core/physics/sphereCollider.h"
#include "game/player.h"
#include "gameObjects/meshObject.h"

// Forward-declare UI::Text to avoid extra include in header
namespace UI {
class Text;
}

class ResourceGenerator : public MeshObject {
public:
	void Start() override;

	void SetGenerationSpeed(float generatedPerSecond);

	size_t GetCurrentlyGenerated() const;

	ResourceType GetResourceType() const;

	void SetResourceType(ResourceType type);

private:
	ResourceType resourceType = ResourceType::Titanium;

	void Interact(std::shared_ptr<Player> player);
	void Hover();
	float lastGenerated = 0;
	// time until Hover() is allowed to show prompt again (session time)
	float hoverDisabledUntil = 0.0f;

	float generatedPerSecond = 1;

	std::weak_ptr<SphereCollider> interactCollider;
};