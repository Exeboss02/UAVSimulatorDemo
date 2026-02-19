#include "game/player.h"

void Player::Start()
{
	this->RigidBody::Start();

	std::vector<std::weak_ptr<GameObject>> children = this->GetChildren();
	for (int i = 0; i < children.size(); i++)
	{
		if(!children[i].expired())
		{
			this->camera = std::dynamic_pointer_cast<CameraObject>(children[i].lock());

			if(!this->camera.expired()) break;
		}
	}

	if(this->camera.expired())
	{
		Logger::Error("Player didn't have camera object!");
		return;
	}

	this->musicTimer.Initialize(2);
	this->sfxTimer.Initialize(0.5f);

	//move listener to audiomanager


	//Master Volume
	AudioManager::GetInstance().SetMasterMusicVolume(0.4f);
	AudioManager::GetInstance().SetMasterSoundEffectsVolume(1);

	//Music
	AudioManager::GetInstance().AddMusicTrackStandardFolder("LethalContact.wav", "contact");

	//SFX
	this->speaker = this->factory->CreateGameObjectOfType<SoundSourceObject>();
	this->speaker.lock()->SetParent(this->GetPtr());
	this->speaker.lock()->SetGain(1.0f);

	//AssetManager::GetInstance().AddSoundClipStandardFolder("Step1.wav", "step1");
	// AssetManager::GetInstance().AddSoundClipStandardFolder("Step2.wav", "step2");
	// AssetManager::GetInstance().AddSoundClipStandardFolder("Step3.wav", "step3");
	
	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Step1.wav"));
	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Step2.wav"));
	this->soundClips.push_back(AssetManager::GetInstance().GetSoundClip("Step3.wav"));
}

void Player::Tick()
{
	this->RigidBody::Tick();

	this->input[0] = this->keyBoardInput.GetMovementVector().data()[0];
	this->input[1] = this->keyBoardInput.GetMovementVector().data()[1];
	this->UpdateCamera();

	float deltaTime = Time::GetInstance().GetDeltaTime();
	if(deltaTime < 1) //to prevent tick spam when loading scene
	{
		this->musicTimer.Tick(deltaTime);

		if(DirectX::XMVectorGetX(DirectX::XMVector3Length(this->moveVector)) > 0.01f)
		{
			this->sfxTimer.Tick(deltaTime);
		}
	}

	if(this->musicTimer.TimeIsUp() && !isPlayingMusic)
	{
		//AudioManager::GetInstance().FadeInPlay("contact", 0, 6);
		AudioManager::GetInstance().Play("contact");
		this->isPlayingMusic = true;
	}

	if(this->sfxTimer.TimeIsUp())
	{
		int randomIndex = RandomInt(0, 2);

		std::shared_ptr<SoundSourceObject> lockedSpeaker = this->speaker.lock();
		lockedSpeaker->SetRandomPitch(0.8f, 1.2f);
		lockedSpeaker->Play(this->soundClips[randomIndex]);

		this->sfxTimer.Reset();
	}
}

void Player::PhysicsTick()
{
	float fixedDeltaTime = Time::GetInstance().GetFixedDeltaTime();

	std::shared_ptr<CameraObject> cam = this->camera.lock();

	if(!cam)
	{
		Logger::Error("Player couldn't find any camera!");
		return;
	}

	this->moveVector = {};
	this->moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(this->GetGlobalRight(), this->input[0] * this->speed * fixedDeltaTime)); //Add x-input
	this->moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(this->GetGlobalForward(), this->input[1] * this->speed * fixedDeltaTime)); //Add z-input

	DirectX::XMStoreFloat3(&this->linearVelocity, this->moveVector);
	this->RigidBody::PhysicsTick(); //has to be last because of gravity
}

void Player::UpdateCamera()
{
	std::shared_ptr<CameraObject> cam = this->camera.lock();

	if (this->keyBoardInput.Quit())
	{
		PostQuitMessage(0);
	}

	// Skip game input if ImGui is capturing mouse or keyboard
	if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
		return;
	}

	if (this->keyBoardInput.Interact()) // 'F'
	{
		this->showCursor = !this->showCursor;
		ShowCursor(this->showCursor);
	}

	if (this->showCursor) {
		std::array<float, 2> lookVector = this->keyBoardInput.GetLookVector();

		static float rot[3] = {0, 0, 0};

		float rotSpeed = this->mouseSensitivity;

		rot[0] += rotSpeed * lookVector[1];
		rot[1] += rotSpeed * lookVector[0];

		if (rot[0] > 1.5f) rot[0] = 1.5f;
		if (rot[0] < -1.5f) rot[0] = -1.5f;

		cam->transform.SetRotationRPY(0.0f, rot[0], 0);
		this->transform.SetRotationRPY(0.0f, 0, rot[1]);
	}
}

void Player::LoadFromJson(const nlohmann::json& data)
{
	this->RigidBody::LoadFromJson(data);

	if(data.contains("speed"))
	{
		this->speed = static_cast<float>(data.at("speed").get<float>());
		Logger::Log("'speed' was found in json: " + std::to_string(this->speed));
	}
	else
	{
		Logger::Log("didn't find 'speed'!!!");
	}

	if(data.contains("mouseSensitivity"))
	{
		this->mouseSensitivity = (float)data.at("mouseSensitivity").get<float>();
		Logger::Log("'mouseSensitivity' was found in json: " + std::to_string(this->mouseSensitivity));
	}
}

void Player::SaveToJson(nlohmann::json& data)
{
	this->RigidBody::SaveToJson(data);

	data["speed"] = this->speed;
	data["mouseSensitivity"] = this->mouseSensitivity;
}
