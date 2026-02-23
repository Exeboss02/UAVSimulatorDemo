#pragma once

#include "utilities/logger.h"
#include <memory>

class GameObject;

template <typename T>
class StaticObject;

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
	/// Creates a new gameobject that should be considerd static after Init has been called or after the StaticObject<T>
	/// goes out of scope. Altering position / rotation / scale or mesh after this will mess with the frustrum culling
	/// and potentially other things in the future.
	/// </summary>
	/// <typeparam name="T">The GameObject type from which to create a StaticObject</typeparam>
	/// <returns>
	/// A StaticObject, you can consider this a promise to add something to the scene, at some
	/// point, either after Init call or When object goes out of scope.
	/// </returns>
	template <typename T>
	StaticObject<T> CreateStaticGameObject();

	/// <summary>
	/// Delete the GameObject after all game logic is done, but before the game renders.
	/// </summary>
	/// <param name="gameObject"></param>
	virtual void QueueDeleteGameObject(std::weak_ptr<GameObject> gameObject) = 0;

	/// <summary>
	/// Get the first object of a type. Returns expired weak_ptr if there are none.
	/// T is any type derived from GameObject.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template <typename T>
	std::weak_ptr<T> FindObjectOfType();

	/// <summary>
	/// Get all objects of a type. Returns empty vector if there are none.
	/// T is any type derived from GameObject.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template <typename T>
	std::vector<std::weak_ptr<T>> FindObjectsOfType();

	virtual ~GameObjectFactory() = default;

	/// <summary>
	/// Gets a unique id, useful for naming gameobjects.
	/// </summary>
	/// <returns></returns>
	virtual int GetNextID() = 0;

	/// <summary>
	/// Get selected GameObject. Used in the Object Hierarchy.
	/// </summary>
	/// <returns></returns>
	virtual std::weak_ptr<GameObject> GetSelected() { return this->selectedObject; }

	/// <summary>
	/// Select GameObject. Used in the Object Hierarchy.
	/// </summary>
	/// <param name="newSelected"></param>
	virtual void SetSelected(std::weak_ptr<GameObject>& newSelected) { this->selectedObject = newSelected; }

	/// <summary>
	/// Load a scene. Remember to use the FilepathHolder for paths!
	/// </summary>
	virtual void QueueLoadScene(std::string filepath) = 0;

	virtual std::string GetMainSceneFilepath() = 0;

private:
	/// <summary>
	/// Add GameObject to game engine logic
	/// </summary>
	/// <param name="gameObject"></param>
	virtual void RegisterGameObject(std::shared_ptr<GameObject> gameObject) = 0;

	// Allow StaticObject<T> to call RegisterGameObject
	template <typename T>
	friend class StaticObject;

protected:
	virtual const std::vector<std::shared_ptr<GameObject>>& GetGameObjects() const = 0;
	std::weak_ptr<GameObject> selectedObject;
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
class StaticObject {

public:
	T* operator->() { return object.get(); }

	/// <summary>
	/// Returns the internal shared_ptr to the object.
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<T> Get() { return object; }

	/// <summary>
	/// Registers the GameObject to the active scene, this would be called automatically when the StaticObject goes out
	/// of scope, but can be called manually to register it sooner if necessary.
	/// </summary>
	void Init() {
		if (!started) {
			std::static_pointer_cast<GameObject>(object)->SetIsStatic(true);
			this->factory->RegisterGameObject(this->object);
			started = true;
		}
	}

	~StaticObject() { this->Init(); }
	operator std::shared_ptr<T>() { return object; }
	operator std::weak_ptr<T>() { return object; }

private:
	StaticObject(std::shared_ptr<T> object, GameObjectFactory* factory) : object(object), factory(factory) {
		std::static_pointer_cast<GameObject>(object)->factory = factory;
		std::static_pointer_cast<GameObject>(object)->myPtr = object;

	}
	std::shared_ptr<T> object;
	GameObjectFactory* factory;
	bool started = false;
	friend class GameObjectFactory;
	friend class GameObject;
};

template <typename T>
StaticObject<T> GameObjectFactory::CreateStaticGameObject() {
	static_assert(std::is_base_of_v<GameObject, T>, "T must derive from GameObject");
	std::shared_ptr<T> object = std::make_shared<T>();
	return StaticObject<T>(object, this);
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
	// Return an expired weak_ptr when nothing is found
	return std::weak_ptr<T>();
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
