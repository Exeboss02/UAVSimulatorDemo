#include "core/physics/rayCaster.h"
#include <limits>

RayCaster::RayCaster() {}

RayCaster::~RayCaster() {}

bool RayCaster::castRay(Ray& ray, RayCastData& rayCastData,
						std::vector<std::weak_ptr<Collider>>& colliders, float maxDistance) {
	int currentClosest = -1;
	float closestDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < colliders.size(); i++) {
		bool didHit = false;
		float distance;
		didHit = colliders[i].lock().get()->IntersectWithRay(ray, distance, maxDistance);

		if (didHit == true && distance < closestDistance) {

			closestDistance = distance;
			currentClosest = i;
			Logger::Log("hit against object nr: ", i, "distance: ", distance);

		}
	}
	if (currentClosest != -1) {
		rayCastData.distance = closestDistance;
		rayCastData.hitColider = colliders[currentClosest];
		return true;
	} 
	else {
		rayCastData.distance = closestDistance;
		return false;
	}
}