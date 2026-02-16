#include "core/physics/testPlayer.h"

TestPlayer::TestPlayer() {}

TestPlayer::~TestPlayer() {}

void TestPlayer::Tick()
{ 
	this->RigidBody::Tick();
	this->gravity = false;

	// this->transform.Rotate(0, deltaTime, 0);

	//PhysicsQueue::GetInstance().SolveCollisions(); //this is extremely temporary
}

void TestPlayer::PhysicsTick()
{
	RigidBody::PhysicsTick();

	float fixedDeltaTime = Time::GetInstance().GetFixedDeltaTime();
	this->moveVector = DirectX::XMFLOAT3(0, 0, 0);
	float speed = 18;

	if (GetAsyncKeyState('I'))
	{
		this->moveVector = FLOAT3ADD(this->moveVector, DirectX::XMFLOAT3(0, 0, speed * fixedDeltaTime));
	}

	if (GetAsyncKeyState('J'))
	{
		this->moveVector = FLOAT3ADD(this->moveVector, DirectX::XMFLOAT3(-speed * fixedDeltaTime, 0, 0));
	}

	if (GetAsyncKeyState('K'))
	{
		this->moveVector = FLOAT3ADD(this->moveVector, DirectX::XMFLOAT3(0, 0, -speed * fixedDeltaTime));
	}

	if (GetAsyncKeyState('L'))
	{
		this->moveVector = FLOAT3ADD(this->moveVector, DirectX::XMFLOAT3(speed * fixedDeltaTime, 0, 0));
	}

	this->linearVelocity = moveVector;
}

void TestPlayer::Start() { this->RigidBody::Start(); }
