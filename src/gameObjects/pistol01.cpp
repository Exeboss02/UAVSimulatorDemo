#include "gameObjects/pistol01.h"
#include <numbers>

Pistol01::Pistol01() {}

Pistol01::~Pistol01() {}


void Pistol01::Start() { 
	
	this->gunVisualPath = "guns/pistol01.glb:Mesh_0";
	this->soundClipShootPath = "Shoot3.wav";
	this->fireRate = 0.3f;
	this->visualScale = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	this->visualRotationRPY = DirectX::XMFLOAT3(0, 0, std::numbers::pi / 2);
	this->muzzlePosition = DirectX::XMFLOAT3(0.0f, 0.15f, 0.5f);
	this->damage = 25;
	
	this->gunPosition = DirectX::XMFLOAT3(-0.4f, -0.4f, 0.7f);
	
	Gun::Start();
}