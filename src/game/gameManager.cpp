#include "game/gameManager.h"
#include "core/filepathHolder.h"

std::weak_ptr<GameManager> GameManager::instance;

GameManager::GameManager() : spawnPoint(DirectX::XMVectorSet(310.0, 5.0, 0.0, 0.0)){}

void GameManager::Start() { 
	auto newPlayer = this->factory->FindObjectOfType<Player>();
	if (!newPlayer.expired()) {
		this->player = newPlayer; 
	} else {
		Logger::Error("Failed to find player, add one to the scene.");
	}

	if (GameManager::instance.expired()) {
		GameManager::instance = std::static_pointer_cast<GameManager>(this->GetPtr());
	} else {
		Logger::Error("Two GameManagers?");
	}
}

void GameManager::Tick() { 
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
