#pragma once 
#include "gameObjects/gameObject3D.h"
#include "game/health.h"
#include "gameObjects/wall.h"
#include "core/audio/soundEngine.h"

class SpaceShip;
struct AStarVertex;

class Cockpit : public GameObject3D {
public:
	void Start() override;
	void SetOnDeath(std::function<void()> func);

	/// <summary>
	/// Sets up pathfinding nodes for a specified room.
	/// </summary>
	/// <param name="spaceShip"></param>
	/// <param name="roomPtr"></param>
	void SetupPathfindingNodes(std::shared_ptr<SpaceShip> spaceShip);

	std::array<std::shared_ptr<AStarVertex>, 2>& GetPathfindingNodes() { return this->pathfindingNodes; }

private:
	std::function<void()> onDeath = []() { Logger::Log("Core Died"); };
	Health health;

	DirectX::XMVECTOR corePosition;

	std::array<std::shared_ptr<AStarVertex>, 2> pathfindingNodes;

	void createVisualsAndColiders();
	std::array<std::weak_ptr<Wall>, 4> walls;

	std::weak_ptr<SoundSourceObject> speaker;
};