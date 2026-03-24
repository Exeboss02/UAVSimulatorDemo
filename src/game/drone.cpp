#include "game/drone.h"
#include "core/input/inputManager.h"
#include "utilities/time.h" // Assuming you have a time manager for DeltaTime

using namespace DirectX;

FPVDrone::FPVDrone() : GameObject3D() {
	velocity = XMVectorZero();
	angularVelocity = XMVectorZero();
}

void FPVDrone::SetInput(float throttle, float roll, float pitch, float yaw) {
	inputThrottle = std::max(0.0f, std::min(1.0f, throttle));
	inputRoll = roll;
	inputPitch = pitch;
	inputYaw = yaw;
}

void FPVDrone::Tick() {
	GameObject3D::Tick();

	InputManager::GetInstance().ReadControllerInput(this->controllerInput->GetControllerIndex());

	// Inside your Game Loop or Player Controller:
	float throttle = this->controllerInput->GetMovementVector()[1]; // Note: Xbox sticks center at 0. Map this to 0-1!
	float yaw = this->controllerInput->GetMovementVector()[0];
	float pitch = this->controllerInput->GetLookVector()[1];
	float roll = this->controllerInput->GetLookVector()[0];

	// Deadzones
	if (abs(yaw) < 0.1f) yaw = 0.0f;
	if (abs(pitch) < 0.1f) pitch = 0.0f;
	if (abs(roll) < 0.1f) roll = 0.0f;

	// Throttle needs to be scaled so bottom is 0.0 and top is 1.0
	float mappedThrottle = (throttle + 1.0f) / 2.0f;

	this->SetInput(mappedThrottle, roll, pitch, yaw);

	// Replace this with your engine's actual DeltaTime fetcher
	float dt = Time::GetInstance().GetDeltaTime(); // Time::GetDeltaTime();
	if (dt <= 0.0f) return;

	// -----------------------------------------------------------
	// 1. FLIGHT CONTROLLER (Acro Mode PID)
	// -----------------------------------------------------------
	// Map inputs to target angular rates (Pitch=X, Yaw=Y, Roll=Z)
	// Note: Yaw is typically slower than Pitch/Roll on real drones
	XMVECTOR targetRates =
		XMVectorSet(inputPitch * maxAcroRate, inputYaw * (maxAcroRate * 0.7f), inputRoll * maxAcroRate, 0.0f);

	// Error = Target - Current
	XMVECTOR error = XMVectorSubtract(targetRates, angularVelocity);

	// PD Controller: Torque = (Error * Kp) - (CurrentRate * Kd)
	XMVECTOR pTerm = XMVectorScale(error, kp);
	XMVECTOR dTerm = XMVectorScale(angularVelocity, kd);
	XMVECTOR torqueCmd = XMVectorSubtract(pTerm, dTerm);

	float tPitch = XMVectorGetX(torqueCmd);
	float tYaw = XMVectorGetY(torqueCmd);
	float tRoll = XMVectorGetZ(torqueCmd);

	// -----------------------------------------------------------
	// 2. MOTOR MIXER (Quad-X Configuration)
	// -----------------------------------------------------------
	float mFR = inputThrottle - tPitch + tYaw - tRoll; // Front-Right
	float mRL = inputThrottle + tPitch + tYaw + tRoll; // Rear-Left
	float mFL = inputThrottle - tPitch - tYaw + tRoll; // Front-Left
	float mRR = inputThrottle + tPitch - tYaw - tRoll; // Rear-Right

	// Clamp motors to physical limits [0% to 100% RPM]
	mFR = std::max(0.0f, std::min(1.0f, mFR));
	mRL = std::max(0.0f, std::min(1.0f, mRL));
	mFL = std::max(0.0f, std::min(1.0f, mFL));
	mRR = std::max(0.0f, std::min(1.0f, mRR));

	// -----------------------------------------------------------
	// 3. LINEAR PHYSICS (Position & Velocity)
	// -----------------------------------------------------------
	float totalThrustNormalized = mFR + mRL + mFL + mRR;
	float totalThrustNewtons = totalThrustNormalized * maxThrustPerMotor;

	// In your engine, {0,1,0} is Up. We rotate the Up vector by the drone's current rotation
	// to get the direction the propellers are pushing.
	XMVECTOR localUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR worldThrust =
		XMVectorScale(XMVector3Rotate(localUp, transform.GetRotationQuaternion()), totalThrustNewtons);

	// Apply Gravity and Air Drag
	XMVECTOR gravityVec = XMVectorSet(0.0f, -gravity * mass, 0.0f, 0.0f);
	XMVECTOR dragVec = XMVectorScale(velocity, -dragCoefficient);

	// F = M * A  =>  A = F / M
	XMVECTOR netForce = XMVectorAdd(XMVectorAdd(worldThrust, gravityVec), dragVec);
	XMVECTOR acceleration = XMVectorScale(netForce, 1.0f / mass);

	// Integrate Velocity and Position
	velocity = XMVectorAdd(velocity, XMVectorScale(acceleration, dt));
	transform.Move(XMVectorScale(velocity, dt));

	// -----------------------------------------------------------
	// 4. ANGULAR PHYSICS (Rotation)
	// -----------------------------------------------------------
	// Convert motor differential thrust directly into angular acceleration for that snappy FPV feel
	XMVECTOR angularAccel = XMVectorSet((mRL + mRR - mFL - mFR) * 20.0f, // Pitch accel (Rear vs Front)
										(mFR + mFL - mRR - mRL) * 10.0f, // Yaw accel (CW vs CCW)
										(mFL + mRL - mFR - mRR) * 20.0f, // Roll accel (Left vs Right)
										0.0f);

	angularVelocity = XMVectorAdd(angularVelocity, XMVectorScale(angularAccel, dt));

	// Apply angular drag so the drone stops rotating instantly when sticks center
	angularVelocity = XMVectorScale(angularVelocity, 1.0f - (5.0f * dt));

	// Integrate angular velocity into a delta Quaternion and apply it to Transform
	// DirectX expects Pitch, Yaw, Roll ordering for this function
	XMVECTOR deltaRotation = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(angularVelocity, dt));
	transform.RotateQuaternion(deltaRotation);
}