#include "game/drone.h"
#include "core/input/inputManager.h"
#include "utilities/time.h" // Assuming you have a time manager for DeltaTime
#include "gameObjects/cameraObject.h"
#include <algorithm>
#include "core/tools.h"
using namespace DirectX;

FPVDrone::FPVDrone() : RigidBody() {
	velocity = XMVectorZero();
	angularVelocity = XMVectorZero();
}

// Helper to convert raw 0-65535 to clean floats
float FPVDrone::NormalizeAxis(DWORD rawValue, const AxisCalibration& cal, bool isThrottle) {
	if (isThrottle) {
		// Map min-max to 0.0f - 1.0f
		float t = static_cast<float>(rawValue - cal.minVal) / static_cast<float>(cal.maxVal - cal.minVal);
		return std::clamp(t, 0.0f, 1.0f);
	} else {
		// Map to -1.0f to 1.0f using the center point
		if (rawValue < cal.centerVal) {
			float t = static_cast<float>(cal.centerVal - rawValue) / static_cast<float>(cal.centerVal - cal.minVal);
			return -std::clamp(t, 0.0f, 1.0f); // -1.0 to 0.0
		} else {
			float t = static_cast<float>(rawValue - cal.centerVal) / static_cast<float>(cal.maxVal - cal.centerVal);
			return std::clamp(t, 0.0f, 1.0f); // 0.0 to 1.0
		}
	}
}

void FPVDrone::SetInput(float throttle, float roll, float pitch, float yaw) {
	inputThrottle = std::max(0.0f, std::min(1.0f, throttle));
	inputRoll = roll;
	inputPitch = pitch;
	inputYaw = yaw;
}

void FPVDrone::PhysicsTick() {

	// 1. Fetch Inputs
	InputManager::GetInstance().ReadControllerInput(this->controllerInput->GetControllerIndex());


	if (this->controllerType == ControllerType::XINPUT) {
		float throttle = this->controllerInput->GetMovementVector()[1];
		float yaw = this->controllerInput->GetMovementVector()[0];
		float pitch = this->controllerInput->GetLookVector()[1];
		float roll = this->controllerInput->GetLookVector()[0];

		// Deadzones
		if (abs(yaw) < 0.1f) yaw = 0.0f;
		if (abs(pitch) < 0.1f) pitch = 0.0f;
		if (abs(roll) < 0.1f) roll = 0.0f;

		// Scale throttle so bottom is 0.0 and top is 1.0
		float mappedThrottle = std::clamp(throttle, 0.0f, 1.0f);
		SetInput(mappedThrottle, roll, pitch, yaw);

	} else if (this->controllerType == ControllerType::RADIOMASTER) {
		JOYINFOEX joyInfo;
		joyInfo.dwSize = sizeof(JOYINFOEX);
		joyInfo.dwFlags = JOY_RETURNALL;

		float rawThrottleFloat = 0.0f;
		float rawRollFloat = 0.0f;
		float rawPitchFloat = 0.0f;
		float rawYawFloat = 0.0f;

		if (joyGetPosEx(JOYSTICKID1, &joyInfo) == JOYERR_NOERROR) {
			// NOTE: This mapping assumes an AETR (Aileron, Elevator, Throttle, Rudder) mixer in EdgeTX.
			// If your drone spins wildly, swap dwXpos, dwYpos, dwZpos, and dwRpos here.
			DWORD rawRoll = joyInfo.dwXpos;		// Aileron
			DWORD rawPitch = joyInfo.dwYpos;	// Elevator
			DWORD rawThrottle = joyInfo.dwZpos; // Throttle
			DWORD rawYaw = joyInfo.dwVpos;		// Rudder

			rawRollFloat = NormalizeAxis(rawRoll, calRoll, false);
			rawPitchFloat = NormalizeAxis(rawPitch, calPitch, false);
			rawYawFloat = NormalizeAxis(rawYaw, calYaw, false);
			rawThrottleFloat = NormalizeAxis(rawThrottle, calThrottle, true);

			// Uncomment to debug which raw axis goes to which variable
			// std::cout << "R:" << rawRoll << " P:" << rawPitch << " T:" << rawThrottle << " Y:" << rawYaw << "\n";
		}

		// Apply Deadzones to spring-loaded sticks to prevent jitter
		float deadzone = 0.02f; // 2% deadzone
		if (abs(rawYawFloat) < deadzone) rawYawFloat = 0.0f;
		if (abs(rawPitchFloat) < deadzone) rawPitchFloat = 0.0f;
		if (abs(rawRollFloat) < deadzone) rawRollFloat = 0.0f;

		SetInput(rawThrottleFloat, rawRollFloat, rawPitchFloat, rawYawFloat);
	}


	// Replace this with your engine's actual Delta Time!
	float dt = Time::GetInstance().GetFixedDeltaTime();
	if (dt <= 0.0f) return;

	// -----------------------------------------------------------
	// 1. FLIGHT CONTROLLER (Acro Mode PID)
	// -----------------------------------------------------------
	XMVECTOR targetRates =
		XMVectorSet(inputPitch * maxAcroRate, inputYaw * (maxAcroRate * 0.7f), inputRoll * maxAcroRate, 0.0f);

	XMVECTOR error = XMVectorSubtract(targetRates, angularVelocity);

	XMVECTOR pTerm = XMVectorScale(error, kp);
	XMVECTOR dTerm = XMVectorScale(angularVelocity, kd);
	XMVECTOR torqueCmd = XMVectorSubtract(pTerm, dTerm);

	float tPitch = XMVectorGetX(torqueCmd);
	float tYaw = XMVectorGetY(torqueCmd);
	float tRoll = XMVectorGetZ(torqueCmd);

	// -----------------------------------------------------------
	// 2. MOTOR MIXER (Quad-X Configuration)
	// -----------------------------------------------------------
	float mFR = inputThrottle - tPitch + tYaw - tRoll;
	float mRL = inputThrottle + tPitch + tYaw + tRoll;
	float mFL = inputThrottle - tPitch - tYaw + tRoll;
	float mRR = inputThrottle + tPitch - tYaw - tRoll;

	Logger::Error("mFR: ", mFR, mRL, mFL, mRR);

	mFR = std::max(0.0f, std::min(1.0f, mFR));
	mRL = std::max(0.0f, std::min(1.0f, mRL));
	mFL = std::max(0.0f, std::min(1.0f, mFL));
	mRR = std::max(0.0f, std::min(1.0f, mRR));

	// -----------------------------------------------------------
	// 3. LINEAR PHYSICS (Position & Velocity)
	// -----------------------------------------------------------
	float totalThrustNormalized = mFR + mRL + mFL + mRR;
	float totalThrustNewtons = totalThrustNormalized * maxThrustPerMotor;

	// this->linearVelocity.x = 0;
	// this->linearVelocity.y = 0;
	// this->linearVelocity.z = 0;
	DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&this->linearVelocity);
	
	// Thrust pushes exactly out of the drone's physical top
	XMVECTOR worldThrust = XMVectorScale(transform.GetGlobalUp(), totalThrustNewtons);

	XMVECTOR gravityVec = XMVectorSet(0.0f, -9.82 * mass, 0.0f, 0.0f);
	XMVECTOR dragVec = XMVectorScale(velocity, -dragCoefficient);

	XMVECTOR netForce = XMVectorAdd(XMVectorAdd(worldThrust, gravityVec), dragVec);
	XMVECTOR acceleration = XMVectorScale(netForce, 1.0f / mass);

	velocity = XMVectorAdd(velocity, XMVectorScale(acceleration, dt));
	//velocity = XMVectorAdd(worldThrust, gravityVec);
	//velocity = gravityVec;
	DirectX::XMFLOAT3 velocityf;
	DirectX::XMStoreFloat3(&velocityf, velocity);

	this->linearVelocity = velocityf;

	//PrintFloat3(this->linearVelocity, "linear velocity last: ");
	//Logger::Error("gravity", std::to_string(worldThrust.m128_f32[0]), ", ", std::to_string(worldThrust.m128_f32[1]), ", ", std::to_string(worldThrust.m128_f32[2]));

	// -----------------------------------------------------------
	// 4. ANGULAR PHYSICS (Rotation)
	// -----------------------------------------------------------
	// FIX 1: Corrected Motor Mixer math (Diagonals for Yaw, Front/Back for Pitch)
	float pitchAccel = (mRL + mRR) - (mFL + mFR); // Rear vs Front
	float yawAccel = (mFR + mRL) - (mFL + mRR);	  // CCW vs CW Diagonals
	float rollAccel = (mFL + mRL) - (mFR + mRR);  // Left vs Right

	XMVECTOR angularAccel = XMVectorSet(pitchAccel * 20.0f, yawAccel * 10.0f, rollAccel * 20.0f, 0.0f);

	angularVelocity = XMVectorAdd(angularVelocity, XMVectorScale(angularAccel, dt));
	angularVelocity = XMVectorScale(angularVelocity, 1.0f - (5.0f * dt)); // Angular drag

	// FIX 2: Bulletproof Global Axis Rotation
	// Extract the drone's exact current orientation in the world
	XMVECTOR right = transform.GetGlobalRight();
	XMVECTOR up = transform.GetGlobalUp();
	XMVECTOR forward = transform.GetGlobalForward();

	float dPitch = XMVectorGetX(angularVelocity) * dt;
	float dYaw = XMVectorGetY(angularVelocity) * dt;
	// We negate Roll because DirectX Left-Handed positive Z rotation rolls Left, not Right
	float dRoll = -XMVectorGetZ(angularVelocity) * dt;

	// Create 3 independent quaternions rotating around the True World Axes
	XMVECTOR qPitch = XMQuaternionRotationAxis(right, dPitch);
	XMVECTOR qYaw = XMQuaternionRotationAxis(up, dYaw);
	XMVECTOR qRoll = XMQuaternionRotationAxis(forward, dRoll);

	// Combine them into one global delta rotation
	XMVECTOR qDelta = XMQuaternionMultiply(qPitch, qYaw);
	qDelta = XMQuaternionMultiply(qDelta, qRoll);

	// Apply it to the current rotation
	XMVECTOR currentRot = transform.GetRotationQuaternion();
	XMVECTOR newRot = XMQuaternionMultiply(currentRot, qDelta);
	newRot = XMQuaternionNormalize(newRot);

	transform.SetRotationQuaternion(newRot);

	RigidBody::PhysicsTick();

	//PrintFloat3(this->linearVelocity, "linear velocity last: ");
}

void FPVDrone::Start() { 
	this->RigidBody::Start();
	auto cam = this->factory->CreateGameObjectOfType<CameraObject>().lock(); 
	cam->SetParent(this->GetPtr());
	this->gravity = 0;

	auto collider = this->factory->CreateGameObjectOfType<SphereCollider>().lock();
	collider->SetDynamic(true);
	collider->SetParent(this->GetPtr());
	this->AddColliderChild(collider);
}
