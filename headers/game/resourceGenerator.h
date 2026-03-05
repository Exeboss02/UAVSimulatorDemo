#pragma once
#include "core/physics/sphereCollider.h"
#include "game/player.h"
#include "gameObjects/meshObject.h"
#include "core/audio/soundEngine.h"

// Forward-declare UI::Text to avoid extra include in header
namespace UI {
class Text;
}

class ResourceGenerator : public MeshObject {
	class GeneratorElement {
	public:
		size_t GetCurrentlyGenerated(float lastGenerated) const;
		float GetGenerationSpeed() const;
		void SetGenerationSpeed(float matPerMin);
		size_t TakeCurrentlyGenerated(float lastGenerated);

	private:
		float change = 0;
		float speed = 2;
	};
public:
	void Start() override;

	GeneratorElement titanium;
	GeneratorElement carbonFiber;
	GeneratorElement lubricant;
	GeneratorElement circuits;

private:

	void Interact(std::shared_ptr<Player> player);
	void Hover();
	float lastGenerated = 0;
	// time until Hover() is allowed to show prompt again (session time)
	float hoverDisabledUntil = 0.0f;

	std::weak_ptr<SphereCollider> interactCollider;

	float change = 0;

	std::weak_ptr<SoundSourceObject> speaker;

};