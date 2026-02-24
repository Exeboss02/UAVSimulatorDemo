#include "gameObjects/mine.h"

void Mine::Start() {
    auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
    collider->dynamic = false;
    collider->ignoreTag = static_cast<Tag>(~Tag::ENEMY);
    collider->solid = false;
    collider->SetParent(this->GetPtr());


}