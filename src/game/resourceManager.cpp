#include "game/resourceManager.h"

Resource::Resource(std::string name, size_t startAmount) : name(name), amount(startAmount) {}

std::string Resource::GetName() const { return this->name; }

size_t Resource::GetAmount() const { return this->amount; }

void Resource::SetAmount(size_t amount) { this->amount = amount; }

void Resource::IncrementAmount(size_t amount) { this->amount += amount; }

bool Resource::DecrementAmount(size_t amount) {
	if (amount > this->amount) return false;

	this->amount -= amount;
	return true;
}

bool Resource::AlterAmount(long long amount) {
	if ((static_cast<long long>(this->amount) + amount) < 0) return false;

	this->amount += amount;
	return true;
}

ResourceManager::ResourceManager()
	: titanium("Titanium", 0), lubricant("Lubricant", 0), carbonFiber("Carbon Fiber", 0), circuit("Circuits", 0) {}
