#pragma once

#include "gameObjects/gameObject3D.h"
#include "game/player.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/testEnemy.h"

struct Round {
	size_t enemyCount;
	size_t breachPoints;
};

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
	void SpawnRound(size_t roundIndex);
	void SpawnEnemy(size_t atBreachpoint);
	void EndRound();

	const size_t& GetCurrentRound();

	const float& GetSpawnDelay();
	void SetSpawnDelay(float& newSpawnDelay);

	virtual void SaveToJson(nlohmann::json& data) override;

	/// <summary>
	/// Don't use this in start
	/// </summary>
	static std::shared_ptr<GameManager> GetInstance();

	/// <summary>
	/// Gets a random number between the start and end values
	/// </summary>
	float GetRandom(float startValue, float endValue);

private:
	static std::weak_ptr<GameManager> instance;

	std::weak_ptr<Player> player;
	std::weak_ptr<SpaceShip> spaceship;

	DirectX::XMVECTOR playerSpawnPoint;


	// Round stuff:

	size_t currentRound;
	std::vector<Round> rounds;
	bool inCombat;

	std::vector<std::weak_ptr<GameObject3D>> enemies;

	struct Path {
		std::vector<std::shared_ptr<AStarVertex>> path;
	};

	std::vector<Path> paths;

	float enemySpawnTimer;
	float enemySpawnDelay;

	float idleTimeTimer;
	const float idleTime;

	size_t unspawnedEnemies;
};