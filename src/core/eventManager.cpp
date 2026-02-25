#include "core/eventManager.h"

// std
#include <algorithm>

EventManager::EventManager() : nextHandle(1) {}

EventManager::Handle EventManager::RegisterCallback(ButtonID buttonId, Callback callback) {
	Handle handle = nextHandle.fetch_add(1, std::memory_order_relaxed);
	std::lock_guard<std::mutex> lock(mutex);
	callbacks[buttonId].emplace_back(handle, std::move(callback));
	return handle;
}

EventManager::Handle EventManager::RegisterOnce(ButtonID buttonId, Callback callback) {
	auto handlePtr = std::make_shared<Handle>(0);
	Callback wrapper = [this, buttonId, callback = std::move(callback), handlePtr]() mutable {
		callback();
		this->UnregisterCallback(buttonId, *handlePtr);
	};
	Handle handle = RegisterCallback(buttonId, std::move(wrapper));
	*handlePtr = handle;
	return handle;
}

bool EventManager::UnregisterCallback(ButtonID buttonId, Handle handle) {
	std::lock_guard<std::mutex> lock(mutex);
	auto it = callbacks.find(buttonId);
	if (it == callbacks.end()) return false;
	auto& vec = it->second;
	auto before = vec.size();
	vec.erase(std::remove_if(vec.begin(), vec.end(), [handle](auto& p) { return p.first == handle; }), vec.end());
	auto after = vec.size();
	if (after == 0) callbacks.erase(it);
	return after != before;
}

void EventManager::Trigger(ButtonID buttonId) {
	std::vector<Callback> callbacksToRun;
	{
		std::lock_guard<std::mutex> lock(mutex);
		auto it = callbacks.find(buttonId);
		if (it == callbacks.end()) return;
		callbacksToRun.reserve(it->second.size());
		for (auto& p : it->second)
			callbacksToRun.emplace_back(p.second);
	}
	for (auto& callback : callbacksToRun) {
		callback();
	}
}

void EventManager::ClearCallbacks(ButtonID buttonId) {
	std::lock_guard<std::mutex> lock(mutex);
	callbacks.erase(buttonId);
}

void EventManager::ClearAllCallbacks() {
	std::lock_guard<std::mutex> lock(mutex);
	callbacks.clear();
}

bool EventManager::HasCallbacks(ButtonID buttonId) const {
	std::lock_guard<std::mutex> lock(mutex);
	auto it = callbacks.find(buttonId);
	return it != callbacks.end() && !it->second.empty();
}
