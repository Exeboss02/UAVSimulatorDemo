#pragma once

#include "utilities/logger.h"

#include <iostream>
#include <memory>
#include <vector>

class MeshObject;
class SpotlightObject;
class GameObject;
class PointLightObject;

class RenderQueue {
public:
	RenderQueue(std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue,
				std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue,
				std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue
				
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

private:
	static RenderQueue* instance;

	std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue;
	std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue;
	std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue;
};