#pragma once

#include "gameObjects/gameObjectFactory.h"
#include "imgui.h"
#include "utilities/logger.h"
#include <DirectXMath.h>
#include <array>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

class Scene;
class GameObjectFactory;

class GameObject {
public:
	GameObject();
	virtual ~GameObject() = default;

	const std::vector<std::weak_ptr<GameObject>>& GetChildren() const;
	const int GetChildCount() const;

	const std::weak_ptr<GameObject> GetParent() const;

	/// <summary>
	/// When using this function, the object's current localPosition isn't updated to match it's old worldPosition,
	/// which could lead to unexpected behaviour.
	/// To get desired results, first set parent/children, THEN set position.
	/// </summary>
	/// <param name="newParent"></param>
	virtual void SetParent(std::weak_ptr<GameObject> newParent);

	/// <summary>
	/// Called after the object is instantiated and added to the scene.
	/// Use this rather than the constructor, because the object is actually connected to the Scene here.
	/// </summary>
	virtual void Start();

	/// <summary>
	/// Called every frame
	/// </summary>
	virtual void Tick();

	/// <summary>
	/// Called every frame after every object has executed their Tick()
	/// </summary>
	virtual void LateTick();

	virtual void PhysicsTick();
	virtual void LatePhysicsTick();

	/// <summary>
	/// Called right before the object is destroyed.
	/// </summary>
	virtual void OnDestroy();

	// This should be in Transform but that doesn't work because GameObjects doesn't have transforms, only GameObject3D
	virtual DirectX::XMMATRIX GetGlobalWorldMatrix(bool inverseTranspose) const;

	std::shared_ptr<GameObject> GetPtr();

	virtual void LoadFromJson(const nlohmann::json& data);
	virtual void SaveToJson(nlohmann::json& data);

private:
	/// <summary>
	/// WARNING: Engine only. Do not use for any in-game logic.
	/// </summary>
	/// <param name="newChild"></param>
	void AddChild(std::weak_ptr<GameObject> newChild);

	/// <summary>
	/// Engine only
	/// </summary>
	/// <param name="oldChild"></param>
	void DeleteChild(std::weak_ptr<GameObject> oldChild);

	std::vector<std::weak_ptr<GameObject>> children;
	std::weak_ptr<GameObject> parent;
	// std::weak_ptr<GameObject> weakPtr;

	friend Scene;

	std::weak_ptr<GameObject> myPtr;

protected:
	GameObjectFactory* factory;
};