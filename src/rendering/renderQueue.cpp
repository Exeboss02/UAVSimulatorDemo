#include "rendering/renderQueue.h"
#include "UI/widget.h"
#include "gameObjects/gameObject.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/pointLightObject.h"
#include "gameObjects/spotlightObject.h"
#include "core/filepathHolder.h"

RenderQueue* RenderQueue::instance = nullptr;

RenderQueue::RenderQueue(std::vector<std::weak_ptr<MeshObject>>& meshRenderQueue,
						 std::vector<std::weak_ptr<SpotlightObject>>& lightRenderQueue,
						 std::vector<std::weak_ptr<PointLightObject>>& pointLightRenderQueue,
						 std::vector<std::weak_ptr<UI::Widget>>& uiRenderQueue)
	: meshRenderQueue(meshRenderQueue), lightRenderQueue(lightRenderQueue),
	  pointLightRenderQueue(pointLightRenderQueue), uiRenderQueue(uiRenderQueue) {
	Logger::Log("Initializing RenderQueue.");

	if (instance) {
		Logger::Error("Tried to create another RenderQueue, there can only be one.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	RenderQueue::instance = this;
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

	RenderQueue::instance->meshRenderQueue.push_back(std::static_pointer_cast<MeshObject>(newMeshObject.lock()));
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

	auto light = std::static_pointer_cast<SpotlightObject>(newSpotlightObject.lock());

	instance->lightRenderQueue.push_back(light);
}

void RenderQueue::AddUIWidget(std::weak_ptr<GameObject> newUIWidget) {
	if (newUIWidget.expired()) {
		Logger::Error("Tried to add expired UI widget.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (!instance) {
		Logger::Error("Tried to add UI widget to queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	// Ensure it's a UI::Widget
	if (auto derived = dynamic_cast<UI::Widget*>(newUIWidget.lock().get()); derived == nullptr) {
		Logger::Error("Tried adding something other than a UI::Widget.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	instance->uiRenderQueue.push_back(std::static_pointer_cast<UI::Widget>(newUIWidget.lock()));
}

void RenderQueue::RemoveUIWidget(std::weak_ptr<GameObject> uiWidget) {
	if (uiWidget.expired()) return;

	if (!instance) {
		Logger::Error("Tried to remove UI widget from queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	GameObject* targetPtr = uiWidget.lock().get();

	for (size_t i = 0; i < instance->uiRenderQueue.size(); ++i) {
		if (instance->uiRenderQueue[i].expired()) {
			instance->uiRenderQueue.erase(instance->uiRenderQueue.begin() + i);
			i--;
			continue;
		}

		auto w = instance->uiRenderQueue[i].lock();
		if (w.get() == targetPtr) {
			instance->uiRenderQueue.erase(instance->uiRenderQueue.begin() + i);
			i--;
		}
	}
}

void RenderQueue::ClearUIQueue() {
	if (!instance) {
		Logger::Error("Tried to clear UI queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	instance->uiRenderQueue.clear();
}

void RenderQueue::AddPointLight(std::weak_ptr<GameObject> newPointLight) {
	if (newPointLight.expired()) {
		Logger::Error("Tried to add expired light.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (!instance) {
		Logger::Error("Tried to add light to queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	auto light = std::static_pointer_cast<PointLightObject>(newPointLight.lock());

	instance->pointLightRenderQueue.push_back(light);
}

void RenderQueue::ClearAllQueues() {
	Logger::Log("Clearing render queue...");

	if (!instance) {
		Logger::Error("Tried to remove object from queue, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	instance->meshRenderQueue.clear();
	instance->lightRenderQueue.clear();
	instance->pointLightRenderQueue.clear();
	instance->uiRenderQueue.clear();

	Logger::Log("Clearing render queue successful.");
}

void RenderQueue::ChangeSkybox(std::string filename) 
{ 
	if (!instance) {
		Logger::Error("Tried to change skybox, but RenderQueue is not initialized.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}

	if (instance->newSkyboxCallback) {
		std::string newSkyboxFilename = (FilepathHolder::GetAssetsDirectory() / "skybox" / filename).string();
		instance->newSkyboxCallback(newSkyboxFilename);
	} else {
		Logger::Error("Tried to change skybox, but the callback is not set.");
		throw std::runtime_error("Fatal error in RenderQueue.");
	}
}
