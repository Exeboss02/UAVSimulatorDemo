#include "game/costHandler.h"
#include <sstream>
#include <vector>

CostHandler::CostHandler(size_t titanium, size_t lubricant, size_t carbonFiber, size_t circuit)
	: titanium(titanium), lubricant(lubricant), carbonFiber(carbonFiber), circuit(circuit) {}

CostHandler::~CostHandler() {}



std::string CostHandler::getCostString() {
	std::vector<std::string> parts;

	if (titanium > 0) parts.push_back("Iron: " + std::to_string(titanium));
	if (lubricant > 0) parts.push_back("Lubricant: " + std::to_string(lubricant));
	if (carbonFiber > 0) parts.push_back("Carbon Fiber: " + std::to_string(carbonFiber));
	if (circuit > 0) parts.push_back("Circuits: " + std::to_string(circuit));

	
	if (parts.empty()) {
		return "Free";
	}

	std::ostringstream oss;
	for (size_t i = 0; i < parts.size(); ++i) {
		oss << parts[i];
		if (i < parts.size() - 1) {
			oss << " \n "; 
		}
	}

	return oss.str();
}
// Getters
size_t CostHandler::getTitanium() const { return titanium; }
size_t CostHandler::getLubricant() const { return lubricant; }
size_t CostHandler::getCarbonFiber() const { return carbonFiber; }
size_t CostHandler::getCircuit() const { return circuit; }

// Setters
void CostHandler::setTitanium(size_t value) { titanium = value; }
void CostHandler::setLubricant(size_t value) { lubricant = value; }
void CostHandler::setCarbonFiber(size_t value) { carbonFiber = value; }
void CostHandler::setCircuit(size_t value) { circuit = value; }