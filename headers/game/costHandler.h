#pragma once
#include <string>

class CostHandler {
public:
	CostHandler(size_t titanium, size_t lubricant, size_t carbonFiber, size_t circuit);
	~CostHandler();

	std::string getCostString();

	size_t getTitanium() const;
	size_t getLubricant() const;
	size_t getCarbonFiber() const;
	size_t getCircuit() const;

	void setTitanium(size_t value);
	void setLubricant(size_t value);
	void setCarbonFiber(size_t value);
	void setCircuit(size_t value);

private:
	size_t titanium;
	size_t lubricant;
	size_t carbonFiber;
	size_t circuit;

};


