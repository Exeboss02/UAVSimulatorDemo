#pragma once

#include <DirectXMath.h>

class GameObject3D;

class Transform {
public:
	Transform(GameObject3D* gameObject);

	Transform(GameObject3D* gameObject, DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion = DirectX::XMQuaternionIdentity(),
			  DirectX::XMVECTOR scale = {1, 1, 1});
	Transform(GameObject3D* gameObject, DirectX::XMVECTOR position, float roll, float pitch, float yaw = 0,
			  DirectX::XMVECTOR scale = {1, 1, 1});

	/// <summary>
	/// Sets the position of the transform.
	/// </summary>
	/// <param name="position"></param>
	void SetPosition(DirectX::XMVECTOR position);

	/// <summary>
	/// Sets the position of the transform.
	/// </summary>
	void SetPosition(float x, float y, float z);

	/// <summary>
	/// Sets the rotation of the transform in order roll, pitch, and yaw angles (in radians).
	/// </summary>
	/// <param name="roll">Roll value in radians</param>
	/// <param name="pitch">Pitch value in radians</param>
	/// <param name="yaw">Yaw value in radians</param>
	void SetRotationRPY(float roll, float pitch, float yaw);

	/// <summary>
	/// Sets the rotation of the transform in order roll, pitch, and yaw angles (in radians)
	/// </summary>
	/// <param name="rollPitchYaw">Vector containing roll pitch and yaw in radians</param>
	void SetRotationRPY(DirectX::XMVECTOR rollPitchYaw);

	/// <summary>
	/// Sets the direction of the quaternion, assuming {0, 1, 0} is up
	/// </summary>
	void SetDirection(float x, float y, float z);

	/// <summary>
	/// Sets the direction of the quaternion, assuming {0, 1, 0} is up
	/// </summary>
	void SetDirection(DirectX::XMVECTOR direction);

	/// <summary>
	/// Sets the rotation using the provided DirectX quaternion.
	/// </summary>
	/// <param name="quaternion">A DirectX::XMVECTOR in quaternion format</param>
	void SetRotationQuaternion(DirectX::XMVECTOR quaternion);

	/// <summary>
	/// Sets the transform scale using a scale vector
	/// </summary>
	/// <param name="scale">The scale vector</param>
	void SetScale(DirectX::XMVECTOR scale);

	/// <summary>
	/// Sets the transform scale using a scale vector
	/// </summary>
	void SetScale(float x, float y, float z);

	/// <summary>
	/// Adds the provided vector to the current position
	/// </summary>
	/// <param name="move">The move vector to be applied to position</param>
	void Move(DirectX::XMVECTOR move);

	/// <summary>
	/// Adds direction * speed to the current position, note that delta time is not considered and direction is assumed
	/// to be normalized
	/// </summary>
	/// <param name="direction">Direction vector for move</param>
	/// <param name="speed">Scaling value</param>
	void Move(DirectX::XMVECTOR direction, float speed);

	/// <summary>
	/// Rotates quaternion by the provided roll, pitch, and yaw values (in radians)
	/// </summary>
	/// <param name="rotationX">Roll value</param>
	/// <param name="rotationY">Pitch value</param>
	/// <param name="rotationZ">Yaw value</param>
	void Rotate(float rotationX, float rotationY, float rotationZ = 0);

	/// <summary>
	/// Rotates trasform by the provided quaternion
	/// </summary>
	/// <param name="quaternion"></param>
	void RotateQuaternion(DirectX::XMVECTOR quaternion);

	/// <summary>
	/// Returns the internal position vecotor
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetPosition() const;

	/// <summary>
	/// Returns internal rotation quaternion
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetRotationQuaternion() const;

	/// <summary>
	/// Returns the forward direction vector based on current rotation
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetDirectionVector() const;

	/// <summary>
	/// Returns the right direction vector based on current rotation
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetRightVector() const;

	/// <summary>
	/// Returns the up direction vector based on current rotation
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetUpVector() const;

	/// <summary>
	/// Returns the scale vector for the transform
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetScale() const;

	/// <summary>
	/// Makes the world matrix for transform
	/// </summary>
	/// <returns>Worldmatrix</returns>
	DirectX::XMFLOAT4X4 GetWorldMatrix(bool inverseTranspose) const;

	/// <summary>
	/// Makes the view matrix for transform
	/// </summary>
	/// <returns>ViewMatrix</returns>
	DirectX::XMFLOAT4X4 GetViewMatrix() const;

	/// <summary>
	/// Returns a rotation quaternion based on yaw and pitch, making sure up is up
	/// </summary>
	/// <param name="yawDegrees">Yaw in degrees</param>
	/// <param name="pitchDegrees">Pitch in degrees</param>
	/// <returns></returns>
	static DirectX::XMVECTOR GetCameraRotationQuaternion(float yawDegrees, float pitchDegrees);

	/// <summary>
	/// Get global position as an XMVECTOR
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetGlobalPosition();

	/// <summary>
	/// Get global rotation as a quaternion, in an XMVECTOR
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetGlobalRotation();

	/// <summary>
	/// Get global scale as an XMVECTOR
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetGlobalScale();

	/// <summary>
	/// Get global world matrix recursively, in an XMMATRIX
	/// </summary>
	DirectX::XMMATRIX GetGlobalWorldMatrix(bool inverseTranspose);

	/// <summary>
	/// Maybe misleading name, but returns the forward vector of the object in global space
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetGlobalForward();

	/// <summary>
	/// Maybe misleading name, but returns the right vector of the object in global space
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetGlobalRight();

	/// <summary>
	/// Maybe misleading name, but returns the up vector of the object in global space
	/// </summary>
	/// <returns></returns>
	DirectX::XMVECTOR GetGlobalUp();

	void HasMoved();

private:
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR quaternion;
	DirectX::XMVECTOR scale;

	GameObject3D* myGameObject;

	enum TransformComponent { SCALE, ROTATION, TRANSLATION };

	DirectX::XMVECTOR GetDecomposedWorldMatrix(const TransformComponent& component);

	bool recalculateGlobalWorldMatrix = true;
	bool recalculateGlobalWorldMatrixInverseTransposed = true;
	bool recalculateGlobalPosition = true;
	bool recalculateGlobalRotation = true;
	bool recalculateGlobalScale = true;

	DirectX::XMMATRIX globalWorldMatrix;
	DirectX::XMMATRIX globalWorldMatrixInverseTransposed;
	DirectX::XMVECTOR globalPosition;
	DirectX::XMVECTOR globalRotation;
	DirectX::XMVECTOR globalScale;
};