#pragma once 
#include "core/physics/ray.h"
#include <memory>
#include "core/physics/collider.h"
struct RayCastData {
	std::weak_ptr<Collider> hitColider;
	float distance;
};

class RayCaster {
public:
	RayCaster();
	~RayCaster();
	/// <summary>
	/// call intersect func on coliders and returns true if hit with hit data stored in rayCastData
	/// </summary>
	/// <param name="rayCastData"></param>
	/// <param name="colliders"></param>
	/// <returns></returns>
	bool castRay(Ray& ray,RayCastData& rayCastData, std::vector<std::weak_ptr<Collider>>& colliders, float maxDistance);

	
	private:
};
