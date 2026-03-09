#include "gameObjects/rifle01.h"
#include <numbers>

Rifle01::Rifle01() {}

Rifle01::~Rifle01() {}

void Rifle01::Start() {

	this->gunVisualPath = "guns/GunTest.glb:Mesh_0";
	this->soundClipShootPath = "Shoot3.wav";
	this->fireRate = 0.1f;
	this->visualScale = DirectX::XMFLOAT3(0.007f, 0.007f, 0.007f);
	this->visualRotationRPY = DirectX::XMFLOAT3(0, 0, std::numbers::pi / 2);
	this->muzzlePosition = DirectX::XMFLOAT3(0.0f, 0.11f, 1.0f);
	this->damage = 10;
	this->gunPosition = DirectX::XMFLOAT3(-0.4f, -0.4f, 0.7f);
	this->fireMode = FireMode::AUTOMATIC;
	this->maxBulletSpread = 5.0f;
	this->timeToMaxBulletSpread = 5.0f;

	Gun::Start();
}