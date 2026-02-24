#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class EventManager {
public:
	using Callback = std::function<void()>;
	using Handle = std::size_t;
	using ButtonID = int;

	EventManager();
	~EventManager() = default;

	// Register a repeating callback for the given button id. Returns a handle.
	Handle RegisterCallback(ButtonID buttonId, Callback callback);

	// Register a callback that will be unregistered after the first invocation.
	Handle RegisterOnce(ButtonID buttonId, Callback callback);

	// Unregister a callback by handle. Returns true if removed.
	bool UnregisterCallback(ButtonID buttonId, Handle handle);

	// Trigger all callbacks registered for the given button id.
	void Trigger(ButtonID buttonId);

	void ClearCallbacks(ButtonID buttonId);
	void ClearAllCallbacks();

	bool HasCallbacks(ButtonID buttonId) const;

private:
	std::unordered_map<ButtonID, std::vector<std::pair<Handle, Callback>>> callbacks;
	std::atomic<Handle> nextHandle;
	mutable std::mutex mutex;
};