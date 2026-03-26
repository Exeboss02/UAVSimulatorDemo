#pragma once

#include "core/input/controllerInput.h"
#include "gameObjects/gameObject3D.h"
#include <DirectXMath.h>
#include <algorithm>
#include <memory>
#include "gameObjects/meshObject.h"
#include "game/fpvTarget.h"
#include "core/physics/collision.h"
#include "gameObjects/cameraObject.h"
#include "UI/canvasObject.h"
#include "UI/text.h"


#include <windows.h>
#include <mmsystem.h>

class FPVDrone : public GameObject3D {
public:
	FPVDrone();
	virtual ~FPVDrone() = default;

	virtual void Tick() override;
	virtual void Start() override;
	virtual void PhysicsTick() override;

	/// <summary>
	/// Feed your controller inputs here every frame.
	/// </summary>
	/// <param name="throttle">0.0 to 1.0 (Left Stick Y, from bottom to center)</param>
	/// <param name="roll">-1.0 to 1.0 (Right Stick X)</param>
	/// <param name="pitch">-1.0 to 1.0 (Right Stick Y)</param>
	/// <param name="yaw">-1.0 to 1.0 (Left Stick X)</param>
	void SetInput(float throttle, float roll, float pitch, float yaw);

	
	void SetText(const std::string& text);
	void SetCompletionText(const std::string& text);

	bool targetColliding = false;
private:
	std::weak_ptr<UI::Text> MakeText(const std::string& name, const std::string& text, float x, float y, float width,
					  UI::Anchor anchor);

	void rototatePropelers();

	void ImGui();

	std::weak_ptr<UI::CanvasObject> canvasObj;

	std::weak_ptr<UI::Text> objectiveText;
	std::weak_ptr<UI::Text> objectiveCompletionText;
	std::weak_ptr<UI::Text> speedText;

	float objectiveCompletionTimer = 0;
	float objectiveCompletionMaxTime = 5;





	std::weak_ptr<CameraObject> fpvCamera;
	std::weak_ptr<CameraObject> chaseCamera;

	std::weak_ptr<MeshObject> droneBody;
	std::weak_ptr<MeshObject> propelers[4];

	std::weak_ptr<BoxCollider> droneCollider;


	std::shared_ptr<ControllerInput> controllerInput = std::make_shared<ControllerInput>(0);

	//controller type
	enum ControllerType { XINPUT, RADIOMASTER } controllerType = ControllerType::XINPUT;

	// --- Input State ---
	float inputThrottle = 0.0f;
	float inputRoll = 0.0f;
	float inputPitch = 0.0f;
	float inputYaw = 0.0f;

	// --- RadioMaster Raw Mapping Data ---
	// These define the raw 0-65535 boundaries for your sticks.
	// A center value of 32767 is standard for spring-loaded axes.
	struct AxisCalibration {
		DWORD minVal = 0;
		DWORD centerVal = 32767;
		DWORD maxVal = 65535;
	};

	AxisCalibration calRoll;
	AxisCalibration calPitch;
	AxisCalibration calYaw;
	AxisCalibration calThrottle; // Throttle usually doesn't need a center, just min/max

	// Helper to normalize raw DWORD to -1.0 to 1.0 (or 0.0 to 1.0 for throttle)
	float NormalizeAxis(DWORD rawValue, const AxisCalibration& cal, bool isThrottle);

	// --- Physics State ---
	DirectX::XMVECTOR velocity;
	DirectX::XMVECTOR angularVelocity; // X=Pitch, Y=Yaw, Z=Roll

	// --- Drone Specifications (Tuned for 5-inch racing drone) ---
	float mass = 0.7f;				// kg
	float gravity = 9.81f;			// m/s^2
	float maxThrustPerMotor = 6.0f; // Newtons (Total thrust = 20N)
	float dragCoefficient = 0.3f;	// Air resistance
	float maxAcroRate = 8.0f;		// Max rotation speed in rad/s

	// --- Acro PID Gains ---
	float kp = 4.0f;
	float kd = 0.05f;
};