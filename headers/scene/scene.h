#pragma once

#include "gameObjects/gameObject.h"
#include "gameObjects/gameObjectFactory.h"
#include "utilities/logger.h"
#include <algorithm>
#include <memory>
#include <vector>

class SceneManager;

class Scene : public GameObjectFactory {
public:
	Scene();
	~Scene() = default;

	void SceneTick();

	virtual void RegisterGameObject(std::shared_ptr<GameObject> gameObject) override;

	virtual void QueueDeleteGameObject(std::weak_ptr<GameObject> gameObject) override;

	size_t GetNumberOfGameObjects();

private:
	/// <summary>
	/// Deletes all GameObjects in the deleteQueue (that is, all objects added using QueueDeleteGameObject())
	/// </summary>
	void DeleteDeleteQueue();

	/// <summary>
	/// Used after finishing loading a scene
	/// </summary>
	void CallStartOnAll();

	std::vector<std::shared_ptr<GameObject>> gameObjects;
	std::vector<std::weak_ptr<GameObject>> deleteQueue;

	friend SceneManager;

	virtual const std::vector<std::shared_ptr<GameObject>>& GetGameObjects() const override;

	bool finishedLoading;
};
