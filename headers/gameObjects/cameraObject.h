#pragma once

#include "gameObjects/gameObject3D.h"
#include "utilities/logger.h"
#include <DirectXMath.h>
#include "utilities/time.h"

#include "imgui.h"
#include <string>

class CameraObject : public GameObject3D {
public:
	/// <summary>
	/// For inputting into a constant buffer
	/// </summary>
	struct alignas(16) CameraMatrixContainer {
		/// <summary>
		/// View Projection Matrix of the camera
		/// </summary>
		DirectX::XMFLOAT4X4 viewProjectionMatrix;
		/// <summary>
		/// Camera world position
		/// </summary>
		DirectX::XMVECTOR cameraPosition;
	};

	CameraObject();
	virtual ~CameraObject() = default;

	virtual void Tick() override;
	virtual void LateTick() override;

	virtual void Start() override;

	void SetMainCamera();

	/// <summary>
	/// Set the fov using degrees
	/// </summary>
	/// <param name="fov"></param>
	void SetFov(float fov);

	/// <summary>
	/// Set distance to far plane
	/// </summary>
	/// <param name="farPlane"></param>
	void SetFarPlane(float farPlane);

	/// <summary>
	/// Set distance to near plane
	/// </summary>
	/// <param name="farPlane"></param>
	void SetNearPlane(float nearPlane);

	/// <summary>
	/// Sets the aspect ration for the camera, for example 16 / 9 or 1 / 1
	/// </summary>
	/// <param name="aspectRatio"></param>
	void SetAspectRatio(float aspectRatio);


	CameraObject::CameraMatrixContainer GetCameraMatrix(bool transposeViewProjMult = false);
	static CameraObject& GetMainCamera();

	/// <summary>
	/// Returns FOV in degrees
	/// </summary>
	/// <returns></returns>
	float GetFov() const;

	/// <summary>
	/// Returns Distance to far plane
	/// </summary>
	/// <returns></returns>
	float GetFarPlane() const;

	/// <summary>
	/// Returns Distance to near plane
	/// </summary>
	/// <returns></returns>
	float GetNearPlane() const;

	/// <summary>
	/// returns the aspectratio, typically either 16/9 or 1/1
	/// </summary>
	/// <returns></returns>
	float GetAspectRatio() const;


	virtual void LoadFromJson(const nlohmann::json& data) override;
	virtual void SaveToJson(nlohmann::json& data) override;

	virtual void ShowInHierarchy() override;

	size_t GetCameraId();

private:
	static CameraObject* mainCamera;
	static size_t cameraIdCounter;

	// CameraMatrixContainer cameraMatrix;

	/// <summary>
	/// Field of view (FOV) in degrees
	/// </summary>
	float fieldOfView;

	float aspectRatio;

	float nearPlane;
	float farPlane;

	//void UpdateCameraMatrix();

protected:
	size_t cameraId;
};