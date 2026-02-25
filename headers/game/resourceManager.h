#pragma once

class Resource {
public:
	Resource(std::string name, size_t startAmount);

	std::string GetName() const;
	size_t GetAmount() const;

	void SetAmount(size_t amount);
	void IncrementAmount(size_t amount);

	/// <summary>
	/// Decreases the amount, may fail if stored amount is less than amount, in that case, no changes will be made and
	/// false is returned.
	/// </summary>
	/// <param name="amount"></param>
	/// <returns></returns>
	bool DecrementAmount(size_t amount);
	/// <summary>
	/// Alters the amount, may fail if stored amount - amount < 0, in that case, no changes will be made and
	/// false is returned.
	/// </summary>
	/// <param name="amount"></param>
	/// <returns></returns>
	bool AlterAmount(long long amount);

private:
	std::string name;
	size_t amount;
};

class ResourceManager {
public:
	ResourceManager();
	Resource titanium;
	Resource lubricant;
	Resource carbonFiber;
	Resource circuit;

private:
};