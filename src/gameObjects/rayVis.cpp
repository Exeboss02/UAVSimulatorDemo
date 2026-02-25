#include "gameObjects/rayVis.h"
RayVis::RayVis() {}

RayVis::~RayVis() {}

void RayVis::Tick() {
	
	if (this->timeToDie != -1.0f && Time::GetInstance().GetSessionTime() - this->timeOfCreation > this->timeToDie) {
			this->factory->QueueDeleteGameObject(this->GetPtr());
	}
}

void RayVis::StartDeathTimer(float timeToDeath) {
	this->timeToDie = timeToDeath;
	
	this->timeOfCreation = Time::GetInstance().GetSessionTime();

}