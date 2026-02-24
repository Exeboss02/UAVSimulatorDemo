#pragma once

#include "gameObjects/gameObject3D.h"
#include "game/player.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/testEnemy.h"

class GameManager : public GameObject {
public:
	GameManager();

	void Start() override;
	void Tick() override;

	void ReloadScene();
	void Win();
	void PlayerDied();
	void Loose();

	void SpawnNextRound();
	void SpawnEnemy();
	void EndRound();

	const size_t& GetCurrentRound();

	const float& GetSpawnDelay();
	void SetSpawnDelay(float& newSpawnDelay);

	virtual void SaveToJson(nlohmann::json& data) override;

	/// <summary>
	/// Don't use this in start
	/// </summary>
	static std::shared_ptr<GameManager> GetInstance();

	float GetRandom(float startValue, float endValue);

private:
	std::weak_ptr<Player> player;
	std::weak_ptr<SpaceShip> spaceship;

	static std::weak_ptr<GameManager> instance;
	DirectX::XMVECTOR spawnPoint;

	size_t currentRound;
	size_t lastRound;

	bool inCombat;

	std::vector<std::weak_ptr<GameObject3D>> enemies;

	// Temp
	std::vector<std::shared_ptr<AStarVertex>> path;

	float spawnTimer;
	float spawnDelay;

	size_t unspawnedEnemies;
};