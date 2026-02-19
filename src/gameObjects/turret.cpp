#include "gameObjects/turret.h"
#include "utilities/time.h"
void Turret::Start() { this->MeshObject::Start(); }

void Turret::Tick() { 
	this->MeshObject::Tick();
	// Lowkey illegal considering it is spawned as static
	this->transform.Rotate(0, 1 * Time::GetInstance().GetDeltaTime(), 0);

}
