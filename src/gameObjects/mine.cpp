#include "gameObjects/mine.h"

void Mine::Start() {
    auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
    collider->SetDynamic(false);
    //collider->SetIgnoreTag(~Tag::ENEMY);
    collider->SetParent(this->GetPtr());
	collider->SetSolid(false);
	collider->SetOnCollision([&](std::weak_ptr<GameObject> trigger) { 
        this->OnExplode();
    });
}

void Mine::OnExplode() { Logger::Log("BOOOM"); }