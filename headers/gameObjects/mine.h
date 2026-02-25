#pragma once
#include "gameObjects/meshObject.h"
#include "core/physics/sphereCollider.h"

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

    float damage = 1000;

    float range = 100;

    std::weak_ptr<SphereCollider> collider;
};