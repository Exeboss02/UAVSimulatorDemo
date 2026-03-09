#pragma once

#include "core/tools.h"
#include "game/player.h"
#include "game/storyManager.h"
#include "gameObjects/SpaceShipObj.h"
#include "gameObjects/enemy.h"
#include "gameObjects/gameObject3D.h"
#include "gameObjects/testEnemy.h"

namespace UI {
class Image;
class Text;
class CanvasObject;
} // namespace UI

struct Round {
	size_t enemyCount;
	size_t breachPoints;
	float timeUntilNext = 30;
};

class GameManager : public GameObject {
public:
	GameManager();
	~GameManager();

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
	std::weak_ptr<StoryManager> GetStoryManager();

	bool GetInCombat() const;

	const size_t& GetCurrentRound();

	std::shared_ptr<Player> GetPlayer();
	const std::vector<std::weak_ptr<Enemy>>& GetEnemies();

	const float& GetSpawnDelay();
	void SetSpawnDelay(float& newSpawnDelay);

	DirectX::XMVECTOR GetPlayerSpawnPoint();

	virtual void SaveToJson(nlohmann::json& data) override;

	/// <summary>
	/// Don't use this in start
	/// </summary>
	static std::shared_ptr<GameManager> GetInstance();

	/// <summary>
	/// Gets a random number between the start and end values
	/// </summary>
	float GetRandom(float startValue, float endValue);

	struct StoryChecks {
		bool buildRoom = false;
		bool buildbuildable = false;
	} storyChecks{};

private:
	std::weak_ptr<StoryManager> storyManager;

	static std::weak_ptr<GameManager> instance;

	std::weak_ptr<Player> player;
	std::weak_ptr<SpaceShip> spaceship;

	DirectX::XMVECTOR playerSpawnPoint;

	void AudioHandling();
	std::weak_ptr<SoundSourceObject> shipSpeaker;

	// Round stuff:

	size_t currentRound;
	std::vector<Round> rounds;
	bool inCombat;

	std::vector<std::weak_ptr<Enemy>> enemies;

	struct Path {
		std::vector<std::shared_ptr<AStarVertex>> path;
	};

	std::vector<Path> paths;

	float enemySpawnTimer;
	float enemySpawnDelay;

	float idleTimeTimer;
	const float startIdleTime;

	size_t unspawnedEnemies;

	// Win screen UI
	bool winScreenVisible = false;
	std::weak_ptr<UI::Image> winBackground;
	std::weak_ptr<UI::Text> winTitle;
	std::weak_ptr<UI::Text> winPrompt;
	std::vector<std::weak_ptr<UI::Text>> winStoryLines;
	float winStartTime = 0.0f; // session time when win screen was shown
	bool isPlayingCombatMusic = false;
	bool isPlayingBuildMusic = false;
	bool isFading = false;
	Timer buildMusicWaitTimer;
};