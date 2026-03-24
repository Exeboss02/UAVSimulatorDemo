#pragma once

#include "core/input/controllerInput.h"
#include "gameObjects/gameObject3D.h"
#include <DirectXMath.h>
#include <algorithm>
#include <memory>

class FPVDrone : public GameObject3D {
public:
	FPVDrone();
	virtual ~FPVDrone() = default;

	virtual void Tick() override;
	virtual void Start() override;

	/// <summary>
	/// Feed your controller inputs here every frame.
	/// </summary>
	/// <param name="throttle">0.0 to 1.0 (Left Stick Y, from bottom to center)</param>
	/// <param name="roll">-1.0 to 1.0 (Right Stick X)</param>
	/// <param name="pitch">-1.0 to 1.0 (Right Stick Y)</param>
	/// <param name="yaw">-1.0 to 1.0 (Left Stick X)</param>
	void SetInput(float throttle, float roll, float pitch, float yaw);

private:
	std::shared_ptr<ControllerInput> controllerInput = std::make_shared<ControllerInput>(0);

	// --- Input State ---
	float inputThrottle = 0.0f;
	float inputRoll = 0.0f;
	float inputPitch = 0.0f;
	float inputYaw = 0.0f;

	// --- Physics State ---
	DirectX::XMVECTOR velocity;
	DirectX::XMVECTOR angularVelocity; // X=Pitch, Y=Yaw, Z=Roll

	// --- Drone Specifications (Tuned for 5-inch racing drone) ---
	float mass = 0.6f;				// kg
	float gravity = 9.81f;			// m/s^2
	float maxThrustPerMotor = 5.0f; // Newtons (Total thrust = 20N)
	float dragCoefficient = 0.3f;	// Air resistance
	float maxAcroRate = 12.0f;		// Max rotation speed in rad/s

	// --- Acro PID Gains ---
	float kp = 4.0f;
	float kd = 0.05f;
};