#include "gameObjects/mine.h"
#include "game/gameManager.h"

void Mine::Start() {
    auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
    collider->SetDynamic(false);
    collider->SetIgnoreTag(~Tag::ENEMY);
    collider->SetParent(this->GetPtr());
	collider->SetSolid(false);
	collider->SetOnCollision([&](std::weak_ptr<GameObject> trigger) { 
        this->OnExplode();
    });
	collider->SetOnHit([&](float damadge) { this->OnExplode(); });
}

void Mine::SetRange(float range) { this->range = range; }

void Mine::SetDamage(float damage) { this->damage = damage; }

float Mine::GetDamage() const { return this->damage; }

float Mine::GetRange() const { return this->range; }

void Mine::SetPostExplosion(std::function<void()> func) { this->postExplosion = func; }

void Mine::OnExplode() { 
    auto& enemies = GameManager::GetInstance()->GetEnemies();
	float rangeSquared = this->range * this->range;

    for (auto& enemyWeak : enemies) {
		if (enemyWeak.expired()) continue;
		auto enemy = enemyWeak.lock();
		auto betweenVec =
			DirectX::XMVectorSubtract(this->transform.GetGlobalPosition(), enemy->transform.GetGlobalPosition());
		float distanceSquared = DirectX::XMVector3Dot(betweenVec, betweenVec).m128_f32[0];

        if (distanceSquared > rangeSquared) continue;

        float damage = (1 / distanceSquared) * this->damage;

        enemy->DecrementHealth(damage);
    }

    this->factory->QueueDeleteGameObject(this->GetPtr());

	this->postExplosion();
}