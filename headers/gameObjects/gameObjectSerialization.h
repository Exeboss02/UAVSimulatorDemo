#pragma once

#include "gameObjects/gameObject3D.h"
#include "utilities/logger.h"
#include <iomanip>
#include <nlohmann/json.hpp>

// using json = nlohmann::json;

void to_json(nlohmann::json& j, const GameObject3D& obj);

void from_json(const nlohmann::json& j, GameObject3D& obj);