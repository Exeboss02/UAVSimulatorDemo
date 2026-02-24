#include "game/gameManager.h"
#include "core/filepathHolder.h"
#include <format>

std::weak_ptr<GameManager> GameManager::instance;

GameManager::GameManager() : spawnPoint(DirectX::XMVectorSet(310.0, 5.0, 0.0, 0.0)), currentRound(0), lastRound(10), inCombat(false) {}

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
}

void GameManager::Tick() { 
	if (this->inCombat) {
		for (size_t i = 0; i < this->enemies.size(); i++) {
			if (this->enemies[i].expired()) {
				this->enemies.erase(this->enemies.begin() + i);
				i--;
				continue;
			}
		}

		if (this->enemies.size() <= 0) {
			EndRound();
		}
	}

	ImGui::Begin("Rounds"); 
	ImGui::Text(std::format("Current round: {}", this->currentRound).c_str());
	ImGui::Text(std::format("Enemies: {}", this->enemies.size()).c_str());
	if (ImGui::Button("Next round")) {
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
	lockedPlayer->transform.SetRotationRPY(0,0,0);
	lockedPlayer->SetCameraRotation(0,0,0);
	lockedPlayer->SetPhysicsPosition(this->spawnPoint);
	lockedPlayer->SetPreviousPhysicsPosition(this->spawnPoint);
}

void GameManager::Loose() { 
	Logger::Log("Game over!"); 
	this->ReloadScene();
}

void GameManager::SpawnNextRound() { 
	Logger::Log("New round!"); 
	this->inCombat = true;

	if (auto spaceshipLock = this->spaceship.lock()) {
		//this->path = spaceshipLock->GetPathfinder()->FindPath(roomMesh->GetPathfindingNodes()[0]);
	} else {
		Logger::Error("Failed to create enemy path.");
	}

	for (size_t i = 0; i < 5; i++) {
		auto enemy = this->factory->CreateGameObjectOfType<TestEnemy>();

		if (auto enemyPtr = enemy.lock()) {
			enemyPtr->SetPath(this->path);
		} else {
			Logger::Error("This shouldn't happen.");
		}
	}
}

void GameManager::EndRound() { 
	this->inCombat = false;
	this->currentRound++; 
}

const size_t& GameManager::GetCurrentRound() { return this->currentRound; }

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
