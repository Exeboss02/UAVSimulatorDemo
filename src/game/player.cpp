#include "game/player.h"

void Player::Tick()
{
	this->UpdateCamera();

	float* input = this->keyBoardInput.GetMovementVector().data(); //float2
	float deltaTime = Time::GetInstance().GetDeltaTime();

	std::shared_ptr<CameraObject> cam = this->camera.lock();
	DirectX::XMVECTOR test = {};
	test.m128_f32[0] = 1;

	DirectX::XMVECTOR moveVector = {};
	moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(cam->GetGlobalRight(), input[0] * this->speed * deltaTime)); //Add x-input
	moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(cam->GetGlobalForward(), input[1] * this->speed * deltaTime)); //Add z-input

	DirectX::XMStoreFloat3(&this->linearVelocity, moveVector);

	PhysicsQueue::GetInstance().SolveCollisions(); //this is extremely temporary
	this->RigidBody::Tick(); //Always call this at the end!
}

void Player::Start()
{
	this->RigidBody::Start();

	std::vector<std::weak_ptr<GameObject>> children = this->GetChildren();
	for (int i = 0; i < children.size(); i++)
	{
		if(!children[i].expired())
		{
			this->camera = std::dynamic_pointer_cast<CameraObject>(children[i].lock());

			if(!this->camera.expired()) break;
		}
	}

	if(this->camera.expired())
	{
		Logger::Error("Player didn't have camera object!");
		return;
	}
}

void Player::UpdateCamera()
{
	std::shared_ptr<CameraObject> cam = this->camera.lock();

	if (this->keyBoardInput.Quit())
	{
		PostQuitMessage(0);
	}

	// Skip game input if ImGui is capturing mouse or keyboard
	if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
		return;
	}

	if (this->keyBoardInput.Interact()) // 'F'
	{
		this->showCursor = !this->showCursor;
		ShowCursor(this->showCursor);
	}

	float speed = Time::GetInstance().GetDeltaTime() * 15;

	if (this->showCursor) {
		std::array<float, 2> lookVector = this->keyBoardInput.GetLookVector();

		static float rot[3] = {0, 0, 0};

		float sensitivity = 2.f;
		float rotSpeed = sensitivity * Time::GetInstance().GetDeltaTime();

		rot[0] += rotSpeed * lookVector[1];
		rot[1] += rotSpeed * lookVector[0];

		if (rot[0] > 1.5f) rot[0] = 1.5f;
		if (rot[0] < -1.5f) rot[0] = -1.5f;

		cam->transform.SetRotationRPY(0.0f, rot[0], rot[1]);
	}
}
