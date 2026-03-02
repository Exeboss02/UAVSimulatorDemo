#include "game/player.h"
#include "UI/interactionPrompt.h"
#include "core/physics/sphereCollider.h"
#include "game/gameManager.h"
#include "game/hud.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/mine.h"
#include "gameObjects/pistol01.h"
#include "gameObjects/rayVis.h"
#include "gameObjects/rifle01.h"
#include "gameObjects/room.h"
#include <numbers>

Player::Player() : cameraRotation{0, 0, 0} { this->controllerInput = std::make_shared<ControllerInput>(0); }

Player::~Player() {}

void Player::Start() {
	this->RigidBody::Start();
	this->health.SetMax(100);

	// Set player to spawn point
	DirectX::XMVECTOR spawnPoint = GameManager::GetInstance()->GetPlayerSpawnPoint();
	this->transform.SetPosition(spawnPoint);
	this->transform.SetRotationRPY(0, 0, 0);
	this->SetCameraRotation(0, 0, 0);
	this->SetPhysicsPosition(spawnPoint);
	this->SetPreviousPhysicsPosition(spawnPoint);

	this->resources.titanium.SetAmount(20);

	// adding camera
	auto cameraWeak = this->factory->CreateGameObjectOfType<CameraObject>();
	auto cameraShared = cameraWeak.lock();
	DirectX::XMFLOAT3 pos(0.0f, 1.5f, 0.0f);
	cameraShared->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	cameraShared->SetParent(this->GetPtr());
	cameraShared->SetFarPlane(200);

	this->camera = cameraShared;

	this->storySpeaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->storySpeaker.lock()->SetParent(this->GetPtr());

	// adding gun
	this->addGun(Guns::pistol);

	// on hit function
	std::function<void(float)> onHitFunction = [&](float value) { this->OnHit(value); };

	// adding body colliders
	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->SetDynamic(true);
		colliderobj->SetSolid(true);
		DirectX::XMFLOAT3 pos(0.0f, 0.0f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::PLAYER);
		colliderobj->SetName("PlayerCollider " + std::to_string(this->factory->GetNextID()));
		colliderobj->SetOnHit(onHitFunction);
	}

	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->SetDynamic(true);
		colliderobj->SetSolid(true);
		DirectX::XMFLOAT3 pos(0.0f, 0.4f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::PLAYER);
		colliderobj->SetName("PlayerCollider " + std::to_string(this->factory->GetNextID()));
		colliderobj->SetOnHit(onHitFunction);
	}

	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->SetDynamic(true);
		colliderobj->SetSolid(true);
		DirectX::XMFLOAT3 pos(0.0f, 0.8f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::PLAYER);
		colliderobj->SetName("PlayerCollider " + std::to_string(this->factory->GetNextID()));
		colliderobj->SetOnHit(onHitFunction);
	}

	// GroundCheck
	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->SetDynamic(true);
		colliderobj->SetSolid(false);
		DirectX::XMFLOAT3 pos(0.0f, -0.3, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(0.5f, 0.5f, 0.5f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
		colliderobj->SetTag(Tag::PLAYER);
		// colliderobj->SetIgnoreTag(~Tag::FLOOR);
		colliderobj->SetName("PlayerCollider " + std::to_string(this->factory->GetNextID()));
	}

	std::function<void(std::weak_ptr<GameObject3D>)> function = [&](std::weak_ptr<GameObject3D> gameObject3D) {
		this->OnCollision(gameObject3D);

		if (auto mine = dynamic_pointer_cast<Mine>(gameObject3D.lock())) {
			Logger::Log("Hit Mine");
		}
	};
	this->SetAllOnCollisionFunction(function);

	this->sfxTimer.Initialize(0.4f);
	this->gunAnimationTimer.Initialize(0.15f);

	// SFX
	this->walkSpeaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->walkSpeaker.lock()->SetParent(this->GetPtr());
	this->walkSpeaker.lock()->SetGain(0.85f);

	this->jumpSpeaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->jumpSpeaker.lock()->SetParent(this->GetPtr());
	this->jumpSpeaker.lock()->SetGain(0.5f);

	this->hurtSpeaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->hurtSpeaker.lock()->SetParent(this->GetPtr());
	this->hurtSpeaker.lock()->SetGain(0.3f);

	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Step1.wav"));
	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Step2.wav"));
	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Step3.wav"));

	// Create HUD owned by player
	this->hud = std::make_unique<HUD>(this->factory);
	this->hud->Start();
	// Register callback so when HUD hides the quit prompt (e.g. NO), player input/cursor are restored
	this->hud->SetOnQuitPromptHidden([this]() {
		this->SetInputEnabled(true);
		this->SetShowCursor(false);
	});
}

void Player::Tick() {
	this->RigidBody::Tick();

	// this->GunAnimation(); //something is weird with this->canShoot, so do not use this yet

	InputManager::GetInstance().ReadControllerInput(this->controllerInput->GetControllerIndex());
	DirectX::XMVECTOR position = this->transform.GetGlobalPosition();

	AudioManager::GetInstance().SetListenerPosition(position.m128_f32[0], position.m128_f32[1], position.m128_f32[2]);
	DirectX::XMVECTOR forward = this->transform.GetGlobalForward();
	DirectX::XMVECTOR up = this->transform.GetGlobalUp();
	ALfloat listenerOrientation[6] = {-forward.m128_f32[0], -forward.m128_f32[1], -forward.m128_f32[2],
									  up.m128_f32[0],		up.m128_f32[1],		  up.m128_f32[2]};
	AudioManager::GetInstance().SetListenerOrientation(listenerOrientation);

	if (this->inputEnabled) {
		this->input[0] =
			this->keyBoardInput.GetMovementVector().data()[0] + this->controllerInput->GetMovementVector().data()[0];
		this->input[1] =
			this->keyBoardInput.GetMovementVector().data()[1] + this->controllerInput->GetMovementVector().data()[1];
	} else {
		this->input[0] = 0.0f;
		this->input[1] = 0.0f;
	}
	if (this->inputEnabled) {
		this->jumpInput = this->keyBoardInput.Jump() || this->controllerInput->Jump();
	} else {
		this->jumpInput = false;
	}

	if (this->jumpInput && this->isGrounded) {
		this->isJumping = true;
	}

	this->UpdateCamera();
	this->Interact();

	float deltaTime = Time::GetInstance().GetDeltaTime();
	if (deltaTime < 1) // to prevent tick spam when loading scene
	{
		if (this->isGrounded && DirectX::XMVectorGetX(DirectX::XMVector3Length(this->moveVector)) > 0.01f) {
			this->sfxTimer.Tick(deltaTime);
		}
	}

	if (this->sfxTimer.TimeIsUp()) {
		int randomIndex = RandomInt(0, 2);

		std::shared_ptr<SoundSourceObject> lockedSpeaker = this->walkSpeaker.lock();
		lockedSpeaker->SetRandomPitch(0.6f, 1.0f);
		lockedSpeaker->Play(this->soundClips[randomIndex]);

		this->sfxTimer.Reset();
	}

	// Landing sound handling
	if (this->isGrounded) {
		if (!this->hasPlayedLandSound) {
			SoundClip* clip = AssetManager::GetInstance().GetSoundClip("Land.wav");
			this->jumpSpeaker.lock()->SetRandomPitch(0.7f, 1.0f);
			this->jumpSpeaker.lock()->SetGain(0.15f);
			this->jumpSpeaker.lock()->Play(clip);
			this->hasPlayedLandSound = true;
		}
	}

	else if (!this->isGrounded) {
		this->hasPlayedLandSound = false;
	}

	// this->aim();
	this->CheckForTriggerPress();

	// Update HUD with current resources
	if (this->hud) this->hud->Update(this->resources, this->health);

	ImGui::Begin("GameManager testing");
	if (ImGui::Button("Win")) {
		GameManager::GetInstance()->Win();
	}
	if (ImGui::Button("Loose")) {
		GameManager::GetInstance()->Loose();
	}
	if (ImGui::Button("Player died")) {
		GameManager::GetInstance()->PlayerDied();
	}
	ImGui::End();

	this->healthFragment += Time::GetInstance().GetDeltaTime() * this->healthRegenPerMin / 60;

	if (this->healthFragment > 1) {
		int generatedHealth = this->healthFragment;
		this->health.Increment(generatedHealth);
		this->healthFragment -= generatedHealth;
	}

}

void Player::PhysicsTick() {
	float fixedDeltaTime = Time::GetInstance().GetFixedDeltaTime();

	// Logger::Log(std::to_string(this->linearVelocity.x), ", ", std::to_string(this->linearVelocity.y), ", ",
	//			std::to_string(this->linearVelocity.z));

	std::shared_ptr<CameraObject> cam = this->camera.lock();

	if (!cam) {
		Logger::Error("Player couldn't find any camera!");
		return;
	}

	this->linearVelocity.x = 0;
	this->linearVelocity.z = 0;
	if (this->isGrounded) {
		this->linearVelocity.y = 0;
	}

	this->moveVector = {};
	this->moveVector = DirectX::XMVectorAdd(
		moveVector, DirectX::XMVectorScale(this->transform.GetGlobalRight(),
										   this->input[0] * this->speed * fixedDeltaTime)); // Add x-input
	this->moveVector = DirectX::XMVectorAdd(
		moveVector, DirectX::XMVectorScale(this->transform.GetGlobalForward(),
										   this->input[1] * this->speed * fixedDeltaTime)); // Add z-input

	if (this->isJumping) {
		DirectX::XMVECTOR jumpVector = {};
		jumpVector.m128_f32[0] = 0;
		jumpVector.m128_f32[1] = this->jumpForce * fixedDeltaTime;
		jumpVector.m128_f32[2] = 0;

		this->moveVector = DirectX::XMVectorAdd(this->moveVector, jumpVector);
		this->isJumping = false;

		// Play jump sound
		SoundClip* clip = AssetManager::GetInstance().GetSoundClip("Jump.wav");
		this->jumpSpeaker.lock()->SetRandomPitch(0.9f, 1.2f);
		this->jumpSpeaker.lock()->SetGain(0.5f);
		this->jumpSpeaker.lock()->Play(clip);
	}

	DirectX::XMVECTOR linearVelocityVector = {};
	linearVelocityVector = DirectX::XMLoadFloat3(&this->linearVelocity);
	linearVelocityVector = DirectX::XMVectorAdd(linearVelocityVector, moveVector);

	DirectX::XMStoreFloat3(&this->linearVelocity, linearVelocityVector);
	this->RigidBody::PhysicsTick(); // has to be last because of gravity

	// reset isGrounded, this gets set to true in OnCollision
	this->isGrounded = false;
}

void Player::UpdateCamera() {
	std::shared_ptr<CameraObject> cam = this->camera.lock();

	if (this->keyBoardInput.Quit()) {
		this->ShowQuitToMenuPrompt();
	}

	// Skip game input if ImGui is capturing mouse or keyboard
	if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
		return;
	}

	if (this->keyBoardInput.ToggleCamera()) {
		this->showCursor = !this->showCursor;
		ShowCursor(this->showCursor);
		this->canShoot = !this->canShoot;
	}

	if (!this->showCursor) {
		float deltaTime = Time::GetInstance().GetDeltaTime();

		std::array<float, 2> lookVector = this->keyBoardInput.GetLookVector();
		lookVector[0] += this->controllerInput->GetLookVector()[0] * this->stickSensitivity * deltaTime;
		lookVector[1] -= this->controllerInput->GetLookVector()[1] * this->stickSensitivity * deltaTime;

		float rotSpeed = this->mouseSensitivity;

		this->cameraRotation[0] += rotSpeed * lookVector[1];
		this->cameraRotation[1] += rotSpeed * lookVector[0];

		if (this->cameraRotation[0] > 1.5f) this->cameraRotation[0] = 1.5f;
		if (this->cameraRotation[0] < -1.5f) this->cameraRotation[0] = -1.5f;

		cam->transform.SetRotationRPY(0.0f, this->cameraRotation[0], 0);
		this->transform.SetRotationRPY(0.0f, 0, this->cameraRotation[1]);
	}
}

void Player::GunAnimation() {
	// if the player can shoot the gun hasn't been fired and should therefore not move
	if (this->canShoot) {
		this->gunIsReturning = false;
		return;
	}

	float deltaTime = Time::GetInstance().GetDeltaTime();
	this->gunAnimationTimer.Tick(deltaTime);

	if (this->gunAnimationTimer.TimeIsUp()) {
		this->gunIsReturning = true;
		this->gunAnimationTimer.Reset();
	}

	DirectX::XMFLOAT3 gunDefaultPos, gunBackPos;
	DirectX::XMStoreFloat3(&gunDefaultPos, this->gunDefaultPosition);
	DirectX::XMStoreFloat3(&gunBackPos, this->gunBackPosition);

	// gun is flying backwards
	if (!this->gunIsReturning) {
		float lerpValue = (this->gunAnimationTimer.startTime - this->gunAnimationTimer.currentTime) /
						  this->gunAnimationTimer.startTime;
		DirectX::XMFLOAT3 currentPos = FLOAT3LERP(gunDefaultPos, gunBackPos, lerpValue);
		DirectX::XMVECTOR newPos = DirectX::XMLoadFloat3(&currentPos);

		this->gun.lock()->transform.SetPosition(newPos);
	} else {
		float lerpValue = this->gunAnimationTimer.currentTime / this->gunAnimationTimer.startTime;
		DirectX::XMFLOAT3 currentPos = FLOAT3LERP(gunDefaultPos, gunBackPos, lerpValue);
		DirectX::XMVECTOR newPos = DirectX::XMLoadFloat3(&currentPos);

		this->gun.lock()->transform.SetPosition(newPos);
	}
}

void Player::SetCameraRotation(float r, float p, float y) {
	this->cameraRotation[0] = r;
	this->cameraRotation[1] = p;
	this->cameraRotation[2] = y;
}

void Player::SetShowCursor(bool visible) {
	this->showCursor = visible;
	// Force OS cursor state to the requested visibility. ShowCursor uses an internal
	// display counter; call repeatedly until the API reports the desired state.
	if (visible) {
		while (ShowCursor(TRUE) < 0) {
			;
		}
	} else {
		while (ShowCursor(FALSE) >= 0) {
			;
		}
	}
	// when showing cursor, disable shooting
	this->canShoot = !visible;
}

void Player::SetInputEnabled(bool enabled) {
	this->inputEnabled = enabled;
	if (!enabled) {
		this->input[0] = 0.0f;
		this->input[1] = 0.0f;
		this->canShoot = false;
	}
}

void Player::SetHealthRegen(float healthPerMin) { this->healthRegenPerMin = healthPerMin; }

void Player::ShowQuitToMenuPrompt() {
	if (this->hud) {
		// Ensure quit prompt is exclusive: do not open it while any build menu is open
		try {
			auto rooms = this->factory->FindObjectsOfType<Room>();
			for (auto& roomWeak : rooms) {
				if (roomWeak.expired()) continue;
				auto room = roomWeak.lock();
				if (!room) continue;
				if (room->IsBuildMenuOpen()) return;
			}
		} catch (...) {
		}

		this->hud->ShowQuitToMenuPrompt();
		this->SetShowCursor(true);
		this->SetInputEnabled(false);
	}
}

void Player::HideQuitToMenuPrompt() {
	if (this->hud) {
		this->hud->HideQuitToMenuPrompt();
		this->SetInputEnabled(true);
		this->SetShowCursor(false);
	}
}

void Player::DecrementHealth(int hp) {
	this->health.Decrement(hp);
	if (this->health.IsDead()) {
		GameManager::GetInstance()->PlayerDied();
	}
}

void Player::IncrementHealth(int hp) { this->health.Increment(hp); }

int Player::GetHealth() const { return this->health.Get(); }

void Player::OnCollision(std::weak_ptr<GameObject3D> gameObject3D) {
	if (gameObject3D.expired()) return;

	std::string name = gameObject3D.lock()->GetName();
	std::shared_ptr<SpaceShip> spaceShip = std::dynamic_pointer_cast<SpaceShip>(gameObject3D.lock());

	if (spaceShip) {
		this->isGrounded = true;
	}
}

void Player::OnHit(float value) {
	this->DecrementHealth(value);
	if(this->health.Get() <= 0)
	{
		SoundClip* hurtClip = AssetManager::GetInstance().GetSoundClip("DeathDelay.wav");
		this->hurtSpeaker.lock()->SetRandomPitch(0.9f, 1.1f);
		this->hurtSpeaker.lock()->Play(hurtClip);
		return;
	}

	SoundClip* hurtClip = AssetManager::GetInstance().GetSoundClip("DeathScream.wav"); //temp sound clip
	this->hurtSpeaker.lock()->SetRandomPitch(0.9f, 1.1f);
	this->hurtSpeaker.lock()->Play(hurtClip);
}

void Player::LoadFromJson(const nlohmann::json& data) {
	this->RigidBody::LoadFromJson(data);

	if (data.contains("speed")) {
		this->speed = static_cast<float>(data.at("speed").get<float>());
		Logger::Log("'speed' was found in json: " + std::to_string(this->speed));
	} else {
		Logger::Log("didn't find 'speed'!!!");
	}

	if (data.contains("mouseSensitivity")) {
		this->mouseSensitivity = (float) data.at("mouseSensitivity").get<float>();
		Logger::Log("'mouseSensitivity' was found in json: " + std::to_string(this->mouseSensitivity));
	}
}

void Player::SaveToJson(nlohmann::json& data) {
	this->RigidBody::SaveToJson(data);

	data["speed"] = this->speed;
	data["mouseSensitivity"] = this->mouseSensitivity;
}

void Player::Interact() {

	const DirectX::XMVECTOR lookVec = this->camera.lock()->transform.GetGlobalForward();
	const DirectX::XMVECTOR posVec = this->camera.lock()->transform.GetGlobalPosition();

	{
		Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
		RayCastData rayCastData;

		bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData, Tag::INTERACTABLE, Tag::PLAYER,
														  this->interactDistance);
		std::string hitString;
		if (didHit) {

			auto hitCollider = rayCastData.hitColider.lock();
			if (hitCollider && (hitCollider->GetTag() & Tag::INTERACTABLE)) {
				hitCollider->Hover();
				hitString = "hit";
			} else {
				hitString = "miss";
				// Hide any shared interaction prompt when nothing is hovered
				try {
					auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
					if (!promptWeak.expired()) {
						auto prompt = promptWeak.lock();
						if (prompt) prompt->Hide();
					}
				} catch (const std::exception& e) {
					Logger::Error("Player::Interact failed to hide prompt: ", e.what());
				} catch (...) {
					Logger::Error("Player::Interact failed to hide prompt (unknown exception)");
				}
			}

		} else {
			hitString = "miss";
			// Hide any shared interaction prompt when nothing is hovered
			try {
				auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
				if (!promptWeak.expired()) {
					auto prompt = promptWeak.lock();
					if (prompt) prompt->Hide();
				}
			} catch (const std::exception& e) {
				Logger::Error("Player::Interact failed to hide prompt: ", e.what());
			} catch (...) {
				Logger::Error("Player::Interact failed to hide prompt (unknown exception)");
			}
		}

		// Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
	}

	if (this->keyBoardInput.Interact() || this->controllerInput->Interact()) {

		Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
		RayCastData rayCastData;

		bool didHit =
			PhysicsQueue::GetInstance().castRay(ray, rayCastData, ~Tag::NOIGNORE, Tag::PLAYER, this->interactDistance);
		std::string hitString;
		if (didHit) {

			rayCastData.hitColider.lock()->Interact(static_pointer_cast<Player>(this->GetPtr()));
			hitString = "hit";

			// rayVis
			//MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
			//auto colliderobjWeak = this->factory->CreateGameObjectOfType<RayVis>();
			//auto colliderobj = colliderobjWeak.lock();
			//colliderobj->StartDeathTimer(5);
			//colliderobj->SetMesh(meshdata);
			//colliderobj->GetMesh().SetMaterial(
			//	0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
			//colliderobj->SetCastShadow(false);
			//colliderobj->transform.SetPosition(
			//	DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, rayCastData.distance / 2)));
			//colliderobj->transform.SetRotationQuaternion(this->camera.lock()->transform.GetGlobalRotation());
			//DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance / 2);
			//colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
			// end of rayVis
		} else {
			hitString = "miss";
		}

		// Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
	}
}

void Player::CheckForTriggerPress() {

	if (this->canShoot) {
		this->gun.lock()->Shoot((this->keyBoardInput.LeftClick() || this->controllerInput->RightClick()),
								this->keyBoardInput.LeftDown() || this->controllerInput->RightDown());

		this->gunDefaultPosition = this->transform.GetPosition();
		this->gunBackPosition.m128_f32[1] = 0.2f;
		this->gunBackPosition.m128_f32[1] = 0.4f;
	}
}

void Player::Aim() {
	static bool isAiming = false;
	if (this->keyBoardInput.RightClick() || this->controllerInput->LeftClick() && this->canShoot && !isAiming) {
		DirectX::XMFLOAT3 pos(0.0f, -0.2f, 0.7f);
		this->gun.lock()->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		isAiming = true;
	} else if (this->keyBoardInput.RightClick() || this->controllerInput->LeftClick() && this->canShoot && isAiming) {

		DirectX::XMFLOAT3 pos(-0.4f, -0.4f, 0.7f);
		this->gun.lock()->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		isAiming = false;
	}
}

void Player::addGun(Guns gunType) {

	if (!this->gun.expired()) {
		this->factory->QueueDeleteGameObject(this->gun);
	}

	std::weak_ptr<Gun> gunWeak;

	switch (gunType) {
	case Guns::pistol:
		gunWeak = this->factory->CreateGameObjectOfType<Pistol01>();
		break;
	case Guns::rifle:
		gunWeak = this->factory->CreateGameObjectOfType<Rifle01>();
		break;
	case Guns::none:
		gunWeak = std::weak_ptr<Gun>();
		break;
	default:
		break;
	}
	if (auto gun = gunWeak.lock()) {
		gun->SetParent(this->camera.lock()->GetPtr());
		gun->setParentToShootFrom(this->camera.lock());
		this->gun = gun;
	} else {
		this->gun = gunWeak;
	}

}