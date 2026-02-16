#include "rendering/renderQueue.h"
#include "gameObjects/gameObject.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/spotlightObject.h"

RenderQueue* RenderQueue::instance = nullptr;

RenderQueue::RenderQueue(std::shared_ptr<std::vector<std::weak_ptr<MeshObject>>> meshRenderQueue,
						 std::shared_ptr<std::vector<std::weak_ptr<SpotlightObject>>> lightRenderQueue)
	: meshRenderQueue(meshRenderQueue), lightRenderQueue(lightRenderQueue) {
	Logger::Log("Initializing RenderQueue.");

	if (instance) {
		Logger::Error("Tried to create another RenderQueue, there can only be one.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	instance = this;
}

void RenderQueue::AddMeshObject(std::weak_ptr<GameObject> newMeshObject) {
	if (newMeshObject.expired()) {
		Logger::Error("Tried to add expired object.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (auto derived = dynamic_cast<MeshObject*>(newMeshObject.lock().get()); derived == nullptr) {
		Logger::Error("Tried adding something other than a mesh object.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (!instance) {
		Logger::Error("Tried to add object to queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (!instance->meshRenderQueue) {
		Logger::Error("meshRenderQueue is null.");
		throw std::runtime_error("Fatal error in RenderQueue");
	}

	instance->meshRenderQueue->push_back(std::static_pointer_cast<MeshObject>(newMeshObject.lock()));
}

void RenderQueue::RemoveMeshObject() {
	if (!instance) {
		throw std::runtime_error("Tried to remove object to queue, but RenderQueue is not initialized.");
	}
}

void RenderQueue::AddLightObject(std::weak_ptr<GameObject> newSpotlightObject) {
	if (newSpotlightObject.expired()) {
		Logger::Error("Tried to add expired light.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (!instance) {
		Logger::Error("Tried to add light to queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (!instance->lightRenderQueue) {
		Logger::Error("lightRenderQueue is null.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	instance->lightRenderQueue->push_back(std::static_pointer_cast<SpotlightObject>(newSpotlightObject.lock()));
}

void RenderQueue::ClearAllQueues() {
	Logger::Log("Clearing render queue...");

	if (!instance) {
		Logger::Error("Tried to remove object from queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	instance->meshRenderQueue->clear();
	instance->lightRenderQueue->clear();

	Logger::Log("Clearing render queue successful.");
}
