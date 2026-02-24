#include "game/gameManager.h"
#include "core/filepathHolder.h"
#include <cstdlib>
#include <format>
#include <random>

std::weak_ptr<GameManager> GameManager::instance;

GameManager::GameManager()
	: spawnPoint(DirectX::XMVectorSet(310.0, 5.0, 0.0, 0.0)), currentRound(0), lastRound(10), inCombat(false),
	  spawnTimer(1), unspawnedEnemies(0), spawnDelay(2), idleTime(30), idleTimeTimer(0) 
{}

void GameManager::Start() {
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

	if (GameManager::instance.expired()) {
		GameManager::instance = std::static_pointer_cast<GameManager>(this->GetPtr());
	} else {
		Logger::Error("Two GameManagers?");
	}

	this->idleTimeTimer = this->idleTime;
}

void GameManager::Tick() {
	if (this->inCombat) {
		if (this->unspawnedEnemies > 0) {
			if (this->spawnTimer > 0) {
				this->spawnTimer -= Time::GetInstance().GetDeltaTime();
			} else {
				SpawnEnemy();
				this->spawnTimer = this->spawnDelay;
			}
		}

		for (size_t i = 0; i < this->enemies.size(); i++) {
			if (this->enemies[i].expired()) {
				this->enemies.erase(this->enemies.begin() + i);
				i--;
				continue;
			}
		}

		if (this->unspawnedEnemies <= 0 && this->enemies.size() <= 0) {
			EndRound();
		}
	} else {
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
	ImGui::Text(std::format("Enemies: {}", this->enemies.size()).c_str());
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
	lockedPlayer->transform.SetPosition(this->spawnPoint);
	lockedPlayer->transform.SetRotationRPY(0, 0, 0);
	lockedPlayer->SetCameraRotation(0, 0, 0);
	lockedPlayer->SetPhysicsPosition(this->spawnPoint);
	lockedPlayer->SetPreviousPhysicsPosition(this->spawnPoint);
}

void GameManager::Loose() {
	Logger::Log("Game over!");
	this->ReloadScene();
}

void GameManager::SpawnNextRound() {
	if (this->inCombat) {
		EndRound();
	}

	Logger::Log("New round!");
	this->inCombat = true;

	if (auto spaceshipLock = this->spaceship.lock()) {
		auto rooms = spaceshipLock->GetPlacedRooms();
		if (rooms.size() <= 0) {
			Logger::Error("No rooms");
			throw std::runtime_error("Fatal error in GameManager.");
		}

		Vector2Int selectedRoom{31, 0};
		float longestDistance = 0;

		for (auto& room : rooms) {
			if (room.second * GetRandom(0.1, 1.0) > longestDistance) {
				longestDistance = room.second;
				selectedRoom = room.first;
			}
		}

		this->path = spaceshipLock->GetPathfinder()->FindPath(
			spaceshipLock->GetRoom(selectedRoom.x, selectedRoom.y).lock()->GetPathfindingNodes()[0]);
	} else {
		Logger::Error("Failed to create enemy path.");
	}

	this->unspawnedEnemies = 5;

	this->spawnTimer = 0;
}

void GameManager::SpawnEnemy() {
	auto enemy = this->factory->CreateGameObjectOfType<TestEnemy>();

	if (auto enemyPtr = enemy.lock()) {
		enemyPtr->SetPath(this->path);
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

	if (currentRound < lastRound) {
		this->currentRound++;
		this->idleTimeTimer = this->idleTime;
	} else {
		Win();
	}
}

const size_t& GameManager::GetCurrentRound() { return this->currentRound; }

const float& GameManager::GetSpawnDelay() { return this->spawnDelay; }

void GameManager::SetSpawnDelay(float& newSpawnDelay) { this->spawnDelay = newSpawnDelay; }

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
