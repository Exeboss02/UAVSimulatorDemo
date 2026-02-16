#pragma once
#include <cmath>
#include "DirectXMath.h"
class Vector3D {
private:
	float x;
	float y;
	float z;

public:
	Vector3D(float xValue, float yValue, float zValue); 

	~Vector3D() = default;
	Vector3D(const Vector3D& other) = default;
	Vector3D(const DirectX::XMVECTOR& directXVector);
	Vector3D(const DirectX::XMFLOAT3& directXfloat3);
	Vector3D& operator=(const Vector3D& other) = default;
	Vector3D(Vector3D&& other) = default;
	Vector3D& operator=(Vector3D&& other) = default;

	Vector3D operator+(const Vector3D& right) const; 
	Vector3D operator-(const Vector3D& right) const; 
	float operator*(const Vector3D& right) const;	 // Dot product
	Vector3D operator^(const Vector3D& right) const; // Cross product
	Vector3D operator*(float scalar) const;		 // Scalar mult

	
	float GetX() const;
	float GetY() const;
	float GetZ() const;
	/// <summary>
	/// returns the lenght of the vector
	/// </summary>
	/// <returns></returns>
	float Length() const; 
	/// <summary>
	/// normalizes the vector to a lenght of 1
	/// </summary>
	void Normalize();	  

	std::string GetString();
};