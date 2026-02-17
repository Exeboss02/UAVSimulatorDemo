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
class QuadTree;

class RenderQueue {
public:
	RenderQueue(std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue,
				std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue,
				std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue, QuadTree& staticObjects
				
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

private:
	static RenderQueue* instance;

	std::function<void(std::string)> newSkyboxCallback;

	friend Renderer;
	std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue;
	std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue;
	std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue;
	QuadTree& staticObjects;
};