#include "game/gameManager.h"
#include "core/filepathHolder.h"
#include <cstdlib>
#include <format>
#include <random>

std::weak_ptr<GameManager> GameManager::instance;

GameManager::GameManager()
	: playerSpawnPoint(DirectX::XMVectorSet(310.0, 5.0, 0.0, 0.0)), currentRound(0), inCombat(false),
	  enemySpawnTimer(1), unspawnedEnemies(0), enemySpawnDelay(2), idleTime(30), idleTimeTimer(0) 
{}

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
}

void GameManager::Tick() {
	if (this->inCombat) {

		// Spawning enemies
		if (this->unspawnedEnemies > 0) {
			if (this->enemySpawnTimer > 0) {
				this->enemySpawnTimer -= Time::GetInstance().GetDeltaTime();
			} else {
				size_t cachedUnspawnedEnemies = this->unspawnedEnemies;
				for (size_t i = 0; i < std::min(this->rounds[this->currentRound].breachPoints, cachedUnspawnedEnemies); i++) {
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
			this->idleTimeTimer -= Time::GetInstance().GetDeltaTime();
		} else {
			SpawnNextRound();
		}
	}

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

void GameManager::ReloadScene() {
	std::string mainScene = this->factory->GetMainSceneFilepath();
	if (mainScene != "") {
		this->factory->QueueLoadScene(mainScene);
	} else {
		Logger::Error("Failed to load scene");
	}
}

void GameManager::Win() { Logger::Log("You won!"); }

void GameManager::PlayerDied() {
	Logger::Log("Player died");
	auto lockedPlayer = this->player.lock();
	lockedPlayer->transform.SetPosition(this->playerSpawnPoint);
	lockedPlayer->transform.SetRotationRPY(0, 0, 0);
	lockedPlayer->SetCameraRotation(0, 0, 0);
	lockedPlayer->SetPhysicsPosition(this->playerSpawnPoint);
	lockedPlayer->SetPreviousPhysicsPosition(this->playerSpawnPoint);
}

void GameManager::Loose() {
	Logger::Log("Game over!");
	this->ReloadScene();
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
			Vector2Int selectedRoom{31, 0};
			float longestDistance = 0;

			for (auto& room : rooms) {
				if (canUseSameBreachPoint || selectedRooms.size() <= 0 || std::find(selectedRooms.begin(), selectedRooms.end(),
																	 room.first) == selectedRooms.end()) {
					if (room.second * GetRandom(0.1, 1.0) > longestDistance) {
						longestDistance = room.second;
						selectedRoom = room.first;
					}
				}
			}

			selectedRooms[i] = selectedRoom;

			this->paths.push_back(Path());
			this->paths[i].path = spaceshipLock->GetPathfinder()->FindPath(
				spaceshipLock->GetRoom(selectedRoom.x, selectedRoom.y).lock()->GetPathfindingNodes()[0]);
		}

	} else {
		Logger::Error("Failed to create enemy path.");
	}

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
		Win();
	}
}

bool GameManager::GetInCombat() const { return this->inCombat; }

const size_t& GameManager::GetCurrentRound() { return this->currentRound; }

std::shared_ptr<Player> GameManager::GetPlayer() { return this->player.lock(); }

const std::vector<std::weak_ptr<Enemy>>& GameManager::GetEnemies() { return this->enemies; }

const float& GameManager::GetSpawnDelay() { return this->enemySpawnDelay; }

void GameManager::SetSpawnDelay(float& newSpawnDelay) { this->enemySpawnDelay = newSpawnDelay; }

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
