#include "game/player.h"
#include "core/physics/sphereCollider.h"

void Player::Start()
{
	this->RigidBody::Start();
	 //adding camera
	auto cameraWeak = this->factory->CreateGameObjectOfType<CameraObject>();
	auto cameraShared = cameraWeak.lock();
	DirectX::XMFLOAT3 pos(0.0f, 2.0f, 0.0f);
	cameraShared->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	cameraShared->SetParent(this->GetPtr());

	this->camera = cameraShared;

	//adding body colliders
	{
	auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
	auto colliderobj = colliderobjWeak.lock();
	colliderobj->dynamic = true;
	colliderobj->solid = true;
	DirectX::XMFLOAT3 pos(0.0f, 0.0f, 0.0f);
	colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
	DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
	colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
	colliderobj->SetParent(this->GetPtr());
	}

	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->dynamic = true;
		colliderobj->solid = true;
		DirectX::XMFLOAT3 pos(0.0f, 0.5f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
	}

	{
		auto colliderobjWeak = this->factory->CreateGameObjectOfType<SphereCollider>();
		auto colliderobj = colliderobjWeak.lock();
		colliderobj->dynamic = true;
		colliderobj->solid = true;
		DirectX::XMFLOAT3 pos(0.0f, 1.0f, 0.0f);
		colliderobj->transform.SetPosition(DirectX::XMLoadFloat3(&pos));
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
		colliderobj->SetParent(this->GetPtr());
	}

}

void Player::Tick()
{
	this->RigidBody::Tick();

	this->input[0] = this->keyBoardInput.GetMovementVector().data()[0];
	this->input[1] = this->keyBoardInput.GetMovementVector().data()[1];
	this->UpdateCamera();
	this->shootRay();

	//Logger::Log("linear velocity: " + std::to_string(this->linearVelocity.x) + ", " + std::to_string(this->linearVelocity.y) + ", " + std::to_string(this->linearVelocity.z));
}

void Player::PhysicsTick()
{
	float fixedDeltaTime = Time::GetInstance().GetFixedDeltaTime();

	std::shared_ptr<CameraObject> cam = this->camera.lock();

	if(!cam)
	{
		Logger::Error("Player couldn't find any camera!");
		return;
	}

	DirectX::XMVECTOR moveVector = {};
	moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(this->GetGlobalRight(), this->input[0] * this->speed * fixedDeltaTime)); //Add x-input
	moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(this->GetGlobalForward(), this->input[1] * this->speed * fixedDeltaTime)); //Add z-input

	DirectX::XMStoreFloat3(&this->linearVelocity, moveVector);
	this->RigidBody::PhysicsTick(); //has to be last because of gravity
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

	if (this->showCursor) {
		std::array<float, 2> lookVector = this->keyBoardInput.GetLookVector();

		static float rot[3] = {0, 0, 0};

		float rotSpeed = this->mouseSensitivity;

		rot[0] += rotSpeed * lookVector[1];
		rot[1] += rotSpeed * lookVector[0];

		if (rot[0] > 1.5f) rot[0] = 1.5f;
		if (rot[0] < -1.5f) rot[0] = -1.5f;

		cam->transform.SetRotationRPY(0.0f, rot[0], 0);
		this->transform.SetRotationRPY(0.0f, 0, rot[1]);
	}
}

void Player::LoadFromJson(const nlohmann::json& data)
{
	this->RigidBody::LoadFromJson(data);

	if(data.contains("speed"))
	{
		this->speed = static_cast<float>(data.at("speed").get<float>());
		Logger::Log("'speed' was found in json: " + std::to_string(this->speed));
	}
	else
	{
		Logger::Log("didn't find 'speed'!!!");
	}

	if(data.contains("mouseSensitivity"))
	{
		this->mouseSensitivity = (float)data.at("mouseSensitivity").get<float>();
		Logger::Log("'mouseSensitivity' was found in json: " + std::to_string(this->mouseSensitivity));
	}
}

void Player::SaveToJson(nlohmann::json& data)
{
	this->RigidBody::SaveToJson(data);

	data["speed"] = this->speed;
	data["mouseSensitivity"] = this->mouseSensitivity;
}

void Player::shootRay() {

	const DirectX::XMVECTOR lookVec = this->camera.lock()->GetGlobalForward();
	const DirectX::XMVECTOR posVec = this->camera.lock()->GetGlobalPosition();
	//{
	//	Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
	//	RayCastData rayCastData;
	//	//Logger::Log("shooting ray");
	//	// Logger::Log("shooting ray from pos: ", ray.origin.GetX(), " ", ray.origin.GetY(), " ", ray.origin.GetZ());
	//	// Logger::Log("with direction: ", ray.direction.GetX(), " ", ray.direction.GetY(), " ", ray.direction.GetZ());

	//	bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData);
	//	std::string hitString;
	//	if (didHit) {

	//		std::shared_ptr<GameObject> hitCollider = rayCastData.hitColider.lock();
	//		std::weak_ptr<GameObject> weakParent = hitCollider->GetParent();
	//		if (!weakParent.expired()) {
	//			std::shared_ptr<GameObject> strongParent = weakParent.lock();
	//			strongParent->OnObserve();
	//		}
	//	} else {
	//		hitString = "miss";
	//	}

	//	//Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
	//}
	if (this->keyBoardInput.LeftClick()) {

		Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
		RayCastData rayCastData;
		Logger::Log("shooting ray");
		// Logger::Log("shooting ray from pos: ", ray.origin.GetX(), " ", ray.origin.GetY(), " ", ray.origin.GetZ());
		// Logger::Log("with direction: ", ray.direction.GetX(), " ", ray.direction.GetY(), " ", ray.direction.GetZ());

		bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData);
		std::string hitString;
		if (didHit) {

			std::shared_ptr<GameObject> hitCollider = rayCastData.hitColider.lock();
			std::weak_ptr<GameObject> weakParent = hitCollider->GetParent();
			if (!weakParent.expired()) {
				std::shared_ptr<GameObject> strongParent = weakParent.lock();
				strongParent->OnInteract();
			}
			hitString = "hit";
		} else {
			hitString = "miss";
		}

		Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
	}
}