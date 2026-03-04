#include "gameObjects/mine.h"
#include "UI/interactionPrompt.h"
#include "core/physics/boxCollider.h"
#include "game/gameManager.h"
#include "gameObjects/room.h"

void Mine::Start() {
	auto collider = this->factory->CreateStaticGameObject<SphereCollider>();
	collider->SetDynamic(false);
	collider->SetIgnoreTag(~Tag::ENEMY);
	collider->SetParent(this->GetPtr());
	collider->SetSolid(false);
	collider->SetOnCollision([&](std::weak_ptr<GameObject> trigger) { this->OnExplode(); });

	collider->transform.SetScale(5, 5, 5);

	auto interactCollider = this->factory->CreateStaticGameObject<BoxCollider>();
	interactCollider->SetParent(this->GetPtr());
	interactCollider->SetSolid(false);
	interactCollider->SetTag(Tag::INTERACTABLE | Tag::OBJECT);
	interactCollider->transform.SetScale(2, 1, 2);
	interactCollider->transform.SetPosition(0, 1, 0);
	interactCollider->SetOnInteract([&](std::shared_ptr<Player> player) { this->RemoveInteract(player); });
	interactCollider->SetOnHover([&] { this->HoverRemove(); });

	this->SetMesh(AssetManager::GetInstance().GetMeshObjData("meshes/mine.glb:Mesh_0"));

	SoundClip* clip = AssetManager::GetInstance().GetSoundClip("Build2.wav");
	std::weak_ptr<SoundSourceObject> speaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
	speaker.lock()->SetParent(this->GetPtr());
	speaker.lock()->SetRandomPitch(0.8f, 1.0f);
	speaker.lock()->SetGain(1.0f);
	speaker.lock()->Play(clip);

	this->transform.SetPosition(0, 1.0f, 0);
	this->transform.SetScale(0.5f, 0.5f, 0.5f);

	// this looks crazy, but feels okey
	this->range = 15;
	this->damage = 100;

	this->MeshObject::Start();
}

void Mine::SetRange(float range) { this->range = range; }

void Mine::SetDamage(float damage) { this->damage = damage; }

float Mine::GetDamage() const { return this->damage; }

float Mine::GetRange() const { return this->range; }

void Mine::SetPostExplosion(std::function<void()> func) { this->postExplosion = func; }

void Mine::OnExplode() {
	Logger::Log("Mine Exploded");

	// When mine gets the destroyed it creates a soundSourceObject that keeps living after
	// the mine stops existing that plays the sound, it gets deleted once it finnishes playing
	SoundClip* explosionClip = AssetManager::GetInstance().GetSoundClip("BigExplosion1.wav");
	std::weak_ptr<SoundSourceObject> speaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	speaker.lock()->SetDeleteWhenFinnished(true);
	speaker.lock()->SetPitch(1.0f);
	speaker.lock()->transform.SetPosition(this->transform.GetGlobalPosition());
	speaker.lock()->Play(explosionClip);

	auto& enemies = GameManager::GetInstance()->GetEnemies();
	float rangeSquared = this->range * this->range;

	for (auto& enemyWeak : enemies) {
		if (enemyWeak.expired()) continue;
		auto enemy = enemyWeak.lock();
		auto betweenVec =
			DirectX::XMVectorSubtract(this->transform.GetGlobalPosition(), enemy->transform.GetGlobalPosition());
		float distanceSquared = DirectX::XMVector3Dot(betweenVec, betweenVec).m128_f32[0];
		float distance = sqrtf(distanceSquared);

		if (distanceSquared > rangeSquared) continue;

		float rangeScalar = (distance / 10);

		float damage = (1 / (rangeScalar * rangeScalar)) * this->damage;

		enemy->DecrementHealth(damage);
	}

	this->factory->QueueDeleteGameObject(this->GetPtr());

	this->postExplosion();
}

void Mine::RemoveInteract(std::shared_ptr<Player> /*player*/) {
	if (auto gameManager = GameManager::GetInstance(); gameManager && gameManager->GetInCombat()) {
		return;
	}

	auto parentWeak = this->GetParent();
	if (parentWeak.expired()) return;

	auto room = std::dynamic_pointer_cast<Room>(parentWeak.lock());
	if (!room) return;

	room->RemoveBuiltObject();

	auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
	if (!promptWeak.expired()) {
		auto prompt = promptWeak.lock();
		if (prompt) prompt->Hide();
	}
}

void Mine::HoverRemove() {
	auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
	if (promptWeak.expired()) return;
	auto prompt = promptWeak.lock();
	if (!prompt) return;

	std::string txt = "Press \"R\" to discard mine";
	if (auto gameManager = GameManager::GetInstance(); gameManager && gameManager->GetInCombat()) {
		txt = "Can't remove during attacks";
	}

	prompt->Show(txt, this->transform.GetGlobalPosition());
}