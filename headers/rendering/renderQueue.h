#pragma once

#include "utilities/logger.h"

#include <iostream>
#include <memory>
#include <vector>
#include <functional>

class Renderer;
class MeshObject;
class SpotlightObject;
class GameObject;
class PointLightObject;
namespace UI { class Widget; }
class QuadTree;

class RenderQueue {
public:
	RenderQueue(std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue,
				std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue,
				std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue,
				QuadTree& staticObjects,
				std::vector<std::weak_ptr<UI::Widget>>& uiRenderQueue
				
	);
	~RenderQueue() = default;

	/// <summary>
	/// Add a mesh object to the meshQueue
	/// </summary>
	/// <param name="newMeshObject"></param>
	static void AddMeshObject(std::weak_ptr<GameObject> newMeshObject);

	/// <summary>
	/// Not implemented
	/// </summary>
	static void RemoveMeshObject();

	/// <summary>
	/// Add a UI Widget to the UI render queue
	/// </summary>
	static void AddUIWidget(std::weak_ptr<GameObject> newUIWidget);

	/// <summary>
	/// Remove a specific UI widget from the UI render queue
	/// </summary>
	static void RemoveUIWidget(std::weak_ptr<GameObject> uiWidget);

	/// <summary>
	/// Remove UI widgets and clear
	/// </summary>
	static void ClearUIQueue();

	/// <summary>
	/// Add a light object to the lightQueue
	/// </summary>
	/// <param name="newSpotlightObject"></param>
	static void AddLightObject(std::weak_ptr<GameObject> newSpotlightObject);

	/// <summary>
	/// Add a pointlight object to the lightQueue
	/// </summary>
	/// <param name="newSpotlightObject"></param>
	static void AddPointLight(std::weak_ptr<GameObject> newPointLight);

	/// <summary>
	/// Clears all render queues
	/// </summary>
	static void ClearAllQueues();

	static void ChangeSkybox(std::string filename);

	static void RecalculateStatic();
	static void RecalculateDynamic();

private:
	friend Renderer;

	static RenderQueue* instance;

	std::function<void(std::string)> newSkyboxCallback;

	std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue;
	std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue;
	std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue;
	QuadTree& staticObjects;
	std::vector<std::weak_ptr<UI::Widget>>& uiRenderQueue;

	bool recalculateStatic;
	bool recalculateDynamic;
};