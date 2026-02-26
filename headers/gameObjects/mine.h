#pragma once
#include "gameObjects/meshObject.h"
#include "core/physics/sphereCollider.h"
#include "core/audio/soundEngine.h"

class Mine : public MeshObject {
public:
    void Start() override;

    void SetRange(float range);
    void SetDamage(float damage);

    float GetDamage() const;
	float GetRange() const;

    void SetPostExplosion(std::function<void()> func);

private:
	void OnExplode();

    std::function<void()> postExplosion = [] {};

    float damage = 500;

    float range = 10;

    std::weak_ptr<SphereCollider> collider;
};