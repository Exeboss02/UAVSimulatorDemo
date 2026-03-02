#pragma once 
#include "gameObjects/gameObject3D.h"

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
	float health = 100;

	DirectX::XMVECTOR corePosition;

	std::array<std::shared_ptr<AStarVertex>, 2> pathfindingNodes;
};