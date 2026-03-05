#include "game/gameManager.h"
#include "UI/canvas.h"
#include "UI/canvasObject.h"
#include "UI/image.h"
#include "UI/text.h"
#include "core/filepathHolder.h"
#include "core/input/keyboardInput.h"
#include "rendering/renderQueue.h"
#include "scene/sceneManager.h"
#include <algorithm>
#include <cstdlib>
#include <format>
#include <random>
#include "core/imguiManager.h"

std::weak_ptr<GameManager> GameManager::instance;

GameManager::~GameManager() { this->shipSpeaker.lock()->Stop(); }

GameManager::GameManager()
	: playerSpawnPoint(DirectX::XMVectorSet(140.0, 5.0, -23.0f, 0.0)), currentRound(0), inCombat(false),
	  enemySpawnTimer(1), unspawnedEnemies(0), enemySpawnDelay(2), idleTime(30), idleTimeTimer(0) {}

void GameManager::Start() {
	if (GameManager::instance.expired()) {
		GameManager::instance = std::static_pointer_cast<GameManager>(this->GetPtr());
	} else {
		Logger::Error("Two GameManagers?");
	}

	auto newPlayer = this->factory->FindObjectOfType<Player>();
	if (!newPlayer.expired()) {
		this->player = newPlayer;
	} else {
		Logger::Error("Failed to find player, add one to the scene.");
	}

	auto newSpaceship = this->factory->FindObjectOfType<SpaceShip>();
	if (!newSpaceship.expired()) {
		this->spaceship = newSpaceship;
	} else {
		Logger::Error("Failed to find spaceship, add one to the scene.");
	}

	this->storyManager = this->factory->CreateGameObjectOfType<StoryManager>();
	this->storyManager.lock()->PlayNextStoryPart();

	// Set up rounds
	this->rounds.reserve(10);
	this->rounds.push_back(Round{3, 1});
	this->rounds.push_back(Round{5, 1});
	this->rounds.push_back(Round{7, 1});
	this->rounds.push_back(Round{10, 2});
	this->rounds.push_back(Round{15, 2});
	this->rounds.push_back(Round{20, 2});
	this->rounds.push_back(Round{25, 2});
	this->rounds.push_back(Round{35, 2});
	this->rounds.push_back(Round{50, 3});
	this->rounds.push_back(Round{80, 3});

	this->idleTimeTimer = this->idleTime;

	// Master Volume
	AudioManager::GetInstance().SetMasterMusicVolume(0.5f);
	AudioManager::GetInstance().SetMasterSoundEffectsVolume(0.5f);

	// Battle music
	this->buildMusicWaitTimer.Initialize(5);
	AudioManager::GetInstance().AddMusicTrackStandardFolder("LethalContact.wav", "contact");
	AudioManager::GetInstance().LoopMusicTrack("contact", true);
	AudioManager::GetInstance().SetGain("contact", 0.4f);

	// Main menu music
	AudioManager::GetInstance().AddMusicTrackStandardFolder("CourageDemo.wav", "courage");
	// AudioManager::GetInstance().LoopMusicTrack("courage", true);
	AudioManager::GetInstance().SetGain("courage", 0.4f);

	DirectX::XMVECTOR offset = {};
	offset.m128_f32[1] = -2;
	offset.m128_f32[2] = 26;
	this->shipSpeaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
	this->shipSpeaker.lock()->transform.SetPosition(DirectX::XMVectorAdd(this->GetPlayerSpawnPoint(), offset));
	SoundClip* buildMusic = AssetManager::GetInstance().GetSoundClip("GTAinBerlin.wav");
	this->shipSpeaker.lock()->SetGain(1.0f);
	this->shipSpeaker.lock()->Play(buildMusic);
}

void GameManager::Tick() {
	// If win screen is visible
	if (this->winScreenVisible) {
		KeyboardInput keyboardInput;
		if (keyboardInput.Interact()) {
			if (auto playerPtr = this->player.lock()) {
				playerPtr->SetShowCursor(true);
				playerPtr->SetInputEnabled(false);
			}

			// Clean up win UI widgets so they don't linger after scene unload
			if (!this->winBackground.expired()) {
				if (auto bg = this->winBackground.lock())
					this->factory->QueueDeleteGameObject(std::weak_ptr<GameObject>(bg));
			}
			if (!this->winTitle.expired()) {
				if (auto t = this->winTitle.lock()) this->factory->QueueDeleteGameObject(std::weak_ptr<GameObject>(t));
			}
			if (!this->winPrompt.expired()) {
				if (auto p = this->winPrompt.lock()) this->factory->QueueDeleteGameObject(std::weak_ptr<GameObject>(p));
			}
			for (auto& lineWeak : this->winStoryLines) {
				if (lineWeak.expired()) continue;
				if (auto line = lineWeak.lock()) {
					this->factory->QueueDeleteGameObject(std::weak_ptr<GameObject>(line));
				}
			}
			this->winStoryLines.clear();
			this->winScreenVisible = false;

			if (auto sm = SceneManager::GetActive()) {
				auto path = (FilepathHolder::GetAssetsDirectory() / "scenes" / "MainMenu.scene").string();
				sm->QueueLoadSceneFile(path);
			}
		}
		return;
	}
	if (this->inCombat) {

		// Spawning enemies
		if (this->unspawnedEnemies > 0) {
			if (this->enemySpawnTimer > 0) {
				this->enemySpawnTimer -= Time::GetInstance().GetDeltaTime();
			} else {
				size_t cachedUnspawnedEnemies = this->unspawnedEnemies;
				for (size_t i = 0;
					 i < (std::min) (this->rounds[this->currentRound].breachPoints, cachedUnspawnedEnemies); i++) {
					SpawnEnemy(i);
				}

				this->enemySpawnTimer = this->enemySpawnDelay;
			}
		}

		// Getting rid of dead enemies
		for (size_t i = 0; i < this->enemies.size(); i++) {
			if (this->enemies[i].expired()) {
				this->enemies.erase(this->enemies.begin() + i);
				i--;
				continue;
			}
		}

		// Checking if round is over
		if (this->unspawnedEnemies <= 0 && this->enemies.size() <= 0) {
			EndRound();
		}

	} else if (currentRound < this->rounds.size()) {

		if (this->idleTimeTimer > 0) {
			if (!this->storyManager.expired() && !this->storyManager.lock()->GetStoryPlaying()) {
				this->idleTimeTimer -= Time::GetInstance().GetDeltaTime();
			}
		} else {
			SpawnNextRound();
		}
	}

	if (this->storyManager.expired()) {
		std::string error = "Story Manager expired before it should";
		Logger::Error(error);
		throw std::runtime_error(error);
	}

	// If not in a round currently and we haven't completed all rounds, showTime == True
	bool showTime = !this->GetInCombat() && (this->currentRound != this->rounds.size()) && !this->storyManager.lock()->GetStoryPlaying();

	// Display current round info in player hud
	this->GetPlayer()->hud->SetRoundIndicator(this->rounds.size() - this->GetCurrentRound(), this->idleTimeTimer,
											  showTime);

	if (!DISABLE_IMGUI) {
		ImGui::Begin("Rounds");
		if (this->inCombat) {
			ImGui::Text(std::format("Enemies: {}", this->enemies.size()).c_str());
		} else {
			ImGui::Text(std::format("Idle time: {}", this->idleTimeTimer).c_str());
		}
		ImGui::Text(std::format("Current round: {}", this->currentRound).c_str());

		if (ImGui::Button("Start next round")) {
			SpawnNextRound();
		}
		ImGui::End();
	}

	this->AudioHandling();
}

void GameManager::ReloadScene() {
	std::string mainScene = this->factory->GetMainSceneFilepath();
	if (mainScene != "") {
		this->factory->QueueLoadScene(mainScene);
	} else {
		Logger::Error("Failed to load scene");
	}
}

void GameManager::Win() {
	if (auto playerPtr = this->player.lock()) {

		playerPtr->transform.SetPosition(10000, 10000, 10000);
		playerPtr->SetPhysicsPosition({10000, 10000, 10000});
		playerPtr->SetPreviousPhysicsPosition({10000, 10000, 10000});

		playerPtr->SetInputEnabled(false);
		playerPtr->SetShowCursor(true);
		if (playerPtr->hud) {
			playerPtr->hud->OnDestroy();
		}
		playerPtr->addGun(Player::Guns::none);
	}

	// Try to find the main HUD canvas to place the win overlay on top
	auto canvasWeak = this->factory->FindObjectOfType<UI::CanvasObject>();
	if (canvasWeak.expired()) {
		Logger::Error("No canvas found for win screen");
		this->winScreenVisible = true; // still enter wait state so game stops
		return;
	}

	auto canvasShared = canvasWeak.lock();
	if (!canvasShared) {
		Logger::Error("Failed to lock canvas for win screen");
		this->winScreenVisible = true;
		return;
	}

	float canvasWidth = static_cast<float>(Window::GetCurrentWidth());
	float canvasHeight = static_cast<float>(Window::GetCurrentHeight());
	try {
		if (auto canvasPtr = canvasShared->GetCanvas()) {
			auto sz = canvasPtr->GetSize();
			if (sz.x > 0.0f) canvasWidth = sz.x;
			if (sz.y > 0.0f) canvasHeight = sz.y;
		}
	} catch (...) {
	}

	this->winTitle.reset();

	// Story text lines (one UI text per line)
	{
		this->winStoryLines.clear();
		const std::vector<std::string> lines = {"After pressing the button you are flung into space,",
												"never to be seen again.",
												"The pirates, or rather the pirate king",
												"who controls them never managed to retrieve the",
												"luxurios fermented golden foxcatjelly coffee beans",
												"you stole.",
												"BERTA on the other hand is living the good life,",
												"traveling between planets, sunbathing,",
												"shooting asteroids and the occasional trader.",
												"You know, AI stuff.",
												"But, hey, you managed to escape the pirates,",
												"so, you won?"};

		const float startY = -220.0f;
		const float lineSpacing = 40.0f;
		for (size_t index = 0; index < lines.size(); ++index) {
			auto lineWeak = this->factory->CreateGameObjectOfType<UI::Text>();
			if (lineWeak.expired()) continue;

			auto lineText = lineWeak.lock();
			if (!lineText) continue;

			lineText->SetName("Win_Story_Line_" + std::to_string(index + 1));
			lineText->SetText(lines[index]);
			lineText->SetFontSize(28.0f);
			lineText->SetAnchor(UI::Anchor::MidCenter);
			lineText->SetPosition(UI::Vec2{0.0f, startY + lineSpacing * static_cast<float>(index)});
			lineText->SetVisible(true);
			lineText->SetZIndex(101);

			std::weak_ptr<GameObject> me = canvasShared->GetPtr();
			lineText->SetParent(me);
			canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(lineText));
			RenderQueue::AddUIWidget(lineText);

			this->winStoryLines.push_back(lineText);
		}
	}

	// Continue prompt at the bottom
	{
		auto promptWeak = this->factory->CreateGameObjectOfType<UI::Text>();
		if (!promptWeak.expired()) {
			auto prompt = promptWeak.lock();
			prompt->SetName("Win_Continue_Prompt");
			prompt->SetText("Press \"F\" to continue...");
			prompt->SetFontSize(22.0f);
			prompt->SetAnchor(UI::Anchor::BottomCenter);
			prompt->SetPosition(UI::Vec2{0.0f, -40.0f});
			prompt->SetVisible(true);
			prompt->SetZIndex(101);
			std::weak_ptr<GameObject> me = canvasShared->GetPtr();
			prompt->SetParent(me);
			canvasShared->AddChild(std::static_pointer_cast<UI::Widget>(prompt));
			RenderQueue::AddUIWidget(prompt);
			this->winPrompt = prompt;
		}
	}

	this->winScreenVisible = true;
}

void GameManager::PlayerDied() {
	Logger::Log("Player died");

	// Sorta temp
	this->Loose();
	return;
	//

	auto lockedPlayer = this->player.lock();
	lockedPlayer->transform.SetPosition(this->playerSpawnPoint);
	lockedPlayer->transform.SetRotationRPY(0, 0, 0);
	lockedPlayer->SetCameraRotation(0, 0, 0);
	lockedPlayer->SetPhysicsPosition(this->playerSpawnPoint);
	lockedPlayer->SetPreviousPhysicsPosition(this->playerSpawnPoint);
	lockedPlayer->IncrementHealth(100);
}

void GameManager::Loose() {
	Logger::Log("Game over!");

	this->factory->QueueLoadScene((FilepathHolder::GetAssetsDirectory() / "scenes/MainMenu.scene").string());
	ShowCursor(true);
	// this->ReloadScene();
}

void GameManager::SpawnNextRound() { this->SpawnRound(this->currentRound); }

void GameManager::SpawnRound(size_t roundIndex) {
	if (this->inCombat) {
		EndRound();
	}

	if (roundIndex < 0 || roundIndex > this->rounds.size()) {
		Logger::Error("Invalid round index.");
		return;
	}

	Logger::Log("New round!");
	this->inCombat = true;

	if (auto spaceshipLock = this->spaceship.lock()) {
		bool closedAnyBuildMenu = false;
		for (const auto& [roomPosition, _] : spaceshipLock->GetPlacedRooms()) {
			if (roomPosition.x < 0 || roomPosition.y < 0) continue;
			auto roomWeak = spaceshipLock->GetRoom(static_cast<size_t>(roomPosition.x),
											 static_cast<size_t>(roomPosition.y));
			if (roomWeak.expired()) continue;
			auto room = roomWeak.lock();
			if (!room || !room->IsBuildMenuOpen()) continue;

			room->CloseBuildMenu();
			closedAnyBuildMenu = true;
		}

		if (closedAnyBuildMenu) {
			if (auto playerPtr = this->player.lock()) {
				const bool quitPromptVisible = playerPtr->hud && playerPtr->hud->IsQuitPromptVisible();
				if (!quitPromptVisible) {
					playerPtr->SetShowCursor(false);
					playerPtr->SetInputEnabled(true);
				}
			}
		}

		auto rooms = spaceshipLock->GetPlacedRooms();
		if (rooms.size() <= 0) {
			Logger::Error("No rooms");
			throw std::runtime_error("Fatal error in GameManager.");
		}

		bool canUseSameBreachPoint = this->rounds[roundIndex].breachPoints > rooms.size();

		std::vector<Vector2Int> selectedRooms(this->rounds[roundIndex].breachPoints);

		this->paths.clear();

		// Set pathfinding from each breach point
		for (size_t i = 0; i < this->rounds[roundIndex].breachPoints; i++) {
			Vector2Int selectedRoom{SpaceShip::START_ROOM_X, SpaceShip::START_ROOM_Y};
			float longestDistance = std::numeric_limits<float>::lowest();

			for (auto& room : rooms) {
				if (canUseSameBreachPoint || selectedRooms.size() <= 0 ||
					std::find(selectedRooms.begin(), selectedRooms.end(), room.first) == selectedRooms.end()) {
					if (room.second * GetRandom(0.1, 1.0) > longestDistance) {
						longestDistance = room.second;
						selectedRoom = room.first;
					}
				}
			}

			selectedRooms[i] = selectedRoom;

			this->paths.push_back(Path());
			this->paths[i].path = spaceshipLock->GetPathfinder()->FindPath(
				spaceshipLock->GetRoom(selectedRoom.x, selectedRoom.y).lock()->GetPathfindingNodes()[1]);
		}

	} else {
		Logger::Error("Failed to create enemy path.");
	}

	this->GetPlayer()->hud->SetObjective("Defend against the pirate attack!");

	this->unspawnedEnemies = this->rounds[roundIndex].enemyCount;

	this->enemySpawnTimer = 0;
}

void GameManager::SpawnEnemy(size_t atBreachpoint) {
	auto enemy = this->factory->CreateGameObjectOfType<Enemy>();

	if (auto enemyPtr = enemy.lock()) {
		if (atBreachpoint >= this->paths.size()) {
			Logger::Error("Invalid breachpoint index");
			return;
		}

		enemyPtr->SetPath(this->paths[atBreachpoint].path);
		this->enemies.push_back(enemyPtr);

		if (unspawnedEnemies > 0) {
			this->unspawnedEnemies--;
		}
	} else {
		Logger::Error("This shouldn't happen.");
	}
}

void GameManager::EndRound() {
	Logger::Log("Finished round");
	this->inCombat = false;
	this->currentRound++;
	this->idleTimeTimer = this->idleTime;

	if (this->currentRound >= this->rounds.size()) {
		// Let storymanager do the winning
	}

	if (auto player = this->player.lock()) {
		player->resources.carbonFiber.IncrementAmount(30 + this->currentRound * 5);
		player->resources.titanium.IncrementAmount(30 + this->currentRound * 5);
		player->resources.circuit.IncrementAmount(30 + this->currentRound * 5);
		player->resources.lubricant.IncrementAmount(30 + this->currentRound * 5);
	}

	if (auto sm = this->storyManager.lock()) {
		sm->PlayNextStoryPart();
	}
}

std::weak_ptr<StoryManager> GameManager::GetStoryManager() { return this->storyManager; }

void GameManager::AudioHandling() {
	float deltaTime = Time::GetInstance().GetDeltaTime();

	if (this->inCombat) {
		if (!this->isPlayingCombatMusic) {
			this->shipSpeaker.lock()->Stop();
			AudioManager::GetInstance().Play("contact");
			this->isPlayingCombatMusic = true;
			this->isFading = false;
			this->isPlayingBuildMusic = false;
		}
	} else {
		ALint state = 0;
		AudioManager::GetInstance().GetMusicTrackSourceState("contact", state);

		if (this->currentRound > 0) {
			if (!this->isFading && !this->isPlayingBuildMusic) {
				AudioManager::GetInstance().FadeOutStop("contact", 6);
				this->isFading = true;
				this->isPlayingCombatMusic = false;
				this->isPlayingBuildMusic = false;
				this->shipSpeaker.lock()->SetGain(1.0f);
			}

			if (!this->isPlayingBuildMusic) {
				this->buildMusicWaitTimer.Tick(deltaTime);

				if (this->buildMusicWaitTimer.TimeIsUp()) {
					this->isPlayingBuildMusic = true;
					this->isFading = false;
					this->isPlayingCombatMusic = false;
					this->shipSpeaker.lock()->SetGain(1.0f);
					this->buildMusicWaitTimer.Reset();

					SoundClip* buildMusic = AssetManager::GetInstance().GetSoundClip("GTAinBerlin.wav");
					this->shipSpeaker.lock()->Play(buildMusic);
				}
			}

			if (this->idleTimeTimer <= 8) {
				float gain = this->shipSpeaker.lock()->GetGain();
				gain -= (deltaTime / 5);
				this->shipSpeaker.lock()->SetGain(gain);
			}
		}
	}
}

bool GameManager::GetInCombat() const { return this->inCombat; }

const size_t& GameManager::GetCurrentRound() { return this->currentRound; }

std::shared_ptr<Player> GameManager::GetPlayer() { return this->player.lock(); }

const std::vector<std::weak_ptr<Enemy>>& GameManager::GetEnemies() { return this->enemies; }

const float& GameManager::GetSpawnDelay() { return this->enemySpawnDelay; }

void GameManager::SetSpawnDelay(float& newSpawnDelay) { this->enemySpawnDelay = newSpawnDelay; }

DirectX::XMVECTOR GameManager::GetPlayerSpawnPoint() { return this->playerSpawnPoint; }

void GameManager::SaveToJson(nlohmann::json& data) {
	this->GameObject::SaveToJson(data);
	data["type"] = "GameManager";
}

std::shared_ptr<GameManager> GameManager::GetInstance() {
	if (!GameManager::instance.expired()) {
		return GameManager::instance.lock();
	} else {
		Logger::Error("Unable to find GameManager");
		throw std::runtime_error("Fatal error in GameManager");
	}
}

float GameManager::GetRandom(float startValue, float endValue) {
	assert(startValue < endValue);

	float diff = endValue - startValue;
	float value = (std::rand() % 10000) / 10000.0;
	value = value * diff + startValue;
	return value;
}
