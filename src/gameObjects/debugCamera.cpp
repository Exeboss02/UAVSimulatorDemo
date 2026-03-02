#include "gameObjects/debugCamera.h"

#include "core/input/inputManager.h"
#include "core/physics/physicsQueue.h"
#include "game/gameManager.h"
#include "imgui.h"
#include "utilities/logger.h"
#include "utilities/time.h"
#include <DirectXMath.h>

void DebugCamera::Tick() {
	this->CameraObject::Tick();

	if (this->cameraId != CameraObject::GetMainCamera().GetCameraId()) return;

	if (keyboardInput.Quit()) {
		try {
			auto player = GameManager::GetInstance()->GetPlayer();
			if (player) {
				player->ShowQuitToMenuPrompt();
			} else {
				PostQuitMessage(0);
			}
		} catch (...) {
			PostQuitMessage(0);
		}
	}

	// Skip game input if ImGui is capturing mouse or keyboard
	if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
		return;
	}

	static bool showCursor = true;

	if (keyboardInput.ToggleCamera()) {
		showCursor = !showCursor;
		ShowCursor(showCursor);
	}

	this->shootRay();

	float speed = Time::GetInstance().GetDeltaTime() * 15;
	this->transform.Move(this->transform.GetDirectionVector(), keyboardInput.GetMovementVector()[1] * speed);
	this->transform.Move(
		DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), this->transform.GetRotationQuaternion()),
		keyboardInput.GetMovementVector()[0] * speed);

	if (!showCursor) {
		std::array<float, 2> lookVector = keyboardInput.GetLookVector();

		float sensitivity = 0.003f;
		float rotSpeed = sensitivity;

		this->rot[0] += rotSpeed * lookVector[1];
		this->rot[1] += rotSpeed * lookVector[0];

		if (this->rot[0] > 1.5f) this->rot[0] = 1.5f;
		if (this->rot[0] < -1.5f) this->rot[0] = -1.5f;

		this->transform.SetRotationRPY(0.0f, this->rot[0], this->rot[1]);
	}

	InputManager::GetInstance().ReadControllerInput(this->controllerInput.GetControllerIndex());

	if (this->controllerInput.Quit()) {
		try {
			auto player = GameManager::GetInstance()->GetPlayer();
			if (player) {
				player->ShowQuitToMenuPrompt();
			} else {
				PostQuitMessage(0);
			}
		} catch (...) {
			PostQuitMessage(0);
		}
	}

	InputManager::GetInstance().ReadControllerInput(this->controllerInput.GetControllerIndex());

	if (this->controllerInput.Quit()) {
		try {
			auto player = GameManager::GetInstance()->GetPlayer();
			if (player) {
				player->ShowQuitToMenuPrompt();
			} else {
				PostQuitMessage(0);
			}
		} catch (...) {
			PostQuitMessage(0);
		}
	}
}

void DebugCamera::shootRay() {
	const DirectX::XMVECTOR lookVec = DirectX::XMVector3Normalize(this->transform.GetDirectionVector());
	const DirectX::XMVECTOR posVec = this->transform.GetPosition();

	if (keyboardInput.LeftClick()) {

		Ray ray{Vector3D{posVec}, Vector3D{lookVec}};
		RayCastData rayCastData;
		Logger::Log("shooting ray");

		bool didHit = PhysicsQueue::GetInstance().castRay(ray, rayCastData);
		std::string hitString;
		if (didHit) {

			// std::shared_ptr<GameObject> hitCollider =
			auto collider = rayCastData.hitColider.lock();
			collider->Interact(nullptr);

			// std::weak_ptr<GameObject> weakParent = hitCollider->GetParent();
			// if (!weakParent.expired()) {
			//	std::shared_ptr<GameObject> strongParent = weakParent.lock();
			//	strongParent->OnInteract();
			// }
			hitString = "hit";

			// rayVis
			MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("TexBox/TextureCube.glb:Mesh_0");
			auto colliderobjWeak = this->factory->CreateGameObjectOfType<MeshObject>();
			auto colliderobj = colliderobjWeak.lock();
			colliderobj->SetMesh(meshdata);
			colliderobj->GetMesh().SetMaterial(
				0, AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial").lock());
			colliderobj->transform.SetPosition(
				DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(lookVec, rayCastData.distance / 2)));
			colliderobj->transform.SetRotationQuaternion(this->transform.GetGlobalRotation());
			DirectX::XMFLOAT3 scale(0.01f, 0.01f, rayCastData.distance / 2);
			colliderobj->transform.SetScale(DirectX::XMLoadFloat3(&scale));
			// end of rayVis
		} else {
			hitString = "miss";
		}

		Logger::Log(hitString, " at distance: ", std::to_string(rayCastData.distance));
	}
}

void DebugCamera::SaveToJson(nlohmann::json& data) {
	this->CameraObject::SaveToJson(data);
	data["type"] = "DebugCamera";
}
