#pragma once

#include "gameObjects/gameObject3D.h"
#include "game/player.h"

class GameManager : public GameObject {
public:
	GameManager();

	void Start() override;
	void Tick() override;

	void ReloadScene();
	void Win();
	void PlayerDied();
	void Loose();

	virtual void SaveToJson(nlohmann::json& data) override;

	/// <summary>
	/// Don't use this in start
	/// </summary>
	static std::shared_ptr<GameManager> GetInstance();

private:
	std::weak_ptr<Player> player;
	static std::weak_ptr<GameManager> instance;
	DirectX::XMVECTOR spawnPoint;
};