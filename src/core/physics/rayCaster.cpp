#include "core/physics/rayCaster.h"
#include <limits>

RayCaster::RayCaster() {}

RayCaster::~RayCaster() {}

bool RayCaster::castRay(Ray& ray, RayCastData& rayCastData, size_t targetTag, size_t tagsToIgnore,
						std::vector<std::weak_ptr<Collider>>& colliders, float maxDistance) 
{
	int currentClosest = -1;
	float closestDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < colliders.size(); i++) {
		std::shared_ptr<Collider> colliderToCheck = colliders[i].lock();
		if (tagsToIgnore & colliderToCheck->GetTag()) {
			continue;
		}
		bool didHit = false;
		float distance;
		didHit = colliderToCheck->IntersectWithRay(ray, distance, maxDistance);

		if (didHit == true && distance < closestDistance) {

			closestDistance = distance;
			currentClosest = i;
			//Logger::Log("hit against object nr: ", i, "distance: ", distance);

		}
	}
	if (currentClosest != -1 && (colliders[currentClosest].lock().get()->GetTag() & targetTag)) {
		rayCastData.distance = closestDistance;
		rayCastData.hitColider = colliders[currentClosest];
		return true;
	} 
	else {
		rayCastData.distance = closestDistance;
		return false;
	}
}