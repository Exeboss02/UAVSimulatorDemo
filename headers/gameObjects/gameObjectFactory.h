#pragma once

#include "utilities/logger.h"
#include <memory>

class GameObject;

class GameObjectFactory {
public:
	/// <summary>
	/// Creates a new GameObject and registers it to the active scene.
	/// T is any type derived from GameObject.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template <typename T>
	std::weak_ptr<T> CreateGameObjectOfType();

	/// <summary>
	/// Delete the GameObject after all game logic is done, but before the game renders.
	/// </summary>
	/// <param name="gameObject"></param>
	virtual void QueueDeleteGameObject(std::weak_ptr<GameObject> gameObject) = 0;

	/// <summary>
	/// Get the first object of a type. Returns expired weak_ptr if there are none.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template <typename T>
	std::weak_ptr<T> FindObjectOfType();

	/// <summary>
	/// Get all objects of a type. Returns empty vector if there are none.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template <typename T>
	std::vector<std::weak_ptr<T>> FindObjectsOfType();

	virtual ~GameObjectFactory() = default;

private:
	/// <summary>
	/// Add GameObject to game engine logic
	/// </summary>
	/// <param name="gameObject"></param>
	virtual void RegisterGameObject(std::shared_ptr<GameObject> gameObject) = 0;

protected:
	virtual const std::vector<std::shared_ptr<GameObject>>& GetGameObjects() const = 0;
};

template <typename T>
inline std::weak_ptr<T> GameObjectFactory::CreateGameObjectOfType() {
	// Make sure it is a gameObject (compiler assert)
	static_assert(std::is_base_of_v<GameObject, T>, "T must derive from GameObject");

	auto obj = std::make_shared<T>();
	RegisterGameObject(obj);
	return obj;
}

template <typename T>
inline std::weak_ptr<T> GameObjectFactory::FindObjectOfType() {

	// Make sure it is a gameObject (compiler assert)
	static_assert(std::is_base_of_v<GameObject, T>, "T must derive from GameObject");

	for (const auto& obj : GetGameObjects()) {
		if (auto casted = std::dynamic_pointer_cast<T>(obj)) {
			return casted;
		}
	}
	Logger::Log("Could not find any object of type T");
	return std::make_shared<T>();
}

template <typename T>
inline std::vector<std::weak_ptr<T>> GameObjectFactory::FindObjectsOfType() {
	// Make sure it is a gameObject (compiler assert)
	static_assert(std::is_base_of_v<GameObject, T>, "T must derive from GameObject");

	std::vector<std::weak_ptr<T>> allObjects;

	for (const auto& obj : GetGameObjects()) {
		if (auto casted = std::dynamic_pointer_cast<T>(obj)) {
			allObjects.push_back(casted);
		}
	}

	return std::move(allObjects);
}
