#pragma once

#include "gameObjects/gameObject.h"
#include "gameObjects/gameObjectFactory.h"
#include "utilities/logger.h"
#include <algorithm>
#include <memory>
#include <vector>

#include "gameObjects/debugCamera.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/spotlightObject.h"

class SceneManager;

class Scene : public GameObjectFactory {
public:
	Scene();
	~Scene() = default;

	/// <summary>
	/// Calls Tick functions on all GameObjects. 
	/// If isPaused is true it means that only debug objects will be called.
	/// </summary>
	/// <param name="isPaused"></param>
	void SceneTick(bool isPaused);

	virtual void RegisterGameObject(std::shared_ptr<GameObject> gameObject) override;

	virtual void QueueDeleteGameObject(std::weak_ptr<GameObject> gameObject) override;

	size_t GetNumberOfGameObjects();

	virtual int GetNextID() override;

private:
	/// <summary>
	/// Deletes all GameObjects in the deleteQueue (that is, all objects added using QueueDeleteGameObject())
	/// </summary>
	void DeleteDeleteQueue();

	/// <summary>
	/// Used after finishing loading a scene
	/// </summary>
	void CallStartOnAll();

	/// <summary>
	/// Creates the object hierarchy window
	/// </summary>
	void ShowHierarchy();

	/// <summary>
	/// Let's all GameObjects add their own stuff to the object hierarchy
	/// </summary>
	/// <param name="name"></param>
	/// <param name="gameObject"></param>
	void ShowHierarchyRecursive(std::weak_ptr<GameObject> gameObject);

	std::vector<std::shared_ptr<GameObject>> gameObjects;
	std::vector<std::weak_ptr<GameObject>> deleteQueue;

	friend SceneManager;

	virtual const std::vector<std::shared_ptr<GameObject>>& GetGameObjects() const override;

	bool finishedLoading; // True if in the middle of loading a scene from file

	int currentGameObjectId; // Keeps track of GameObjectIds
};
