#include "core/physics/vector3D.h"

Vector3D::Vector3D(float xValue, float yValue, float zValue) {
	this->x = xValue;
	this->y = yValue;
	this->z = zValue;
}

Vector3D::Vector3D(const DirectX::XMVECTOR& directXVector) {
	DirectX::XMFLOAT3 components;
	
	DirectX::XMStoreFloat3(&components, directXVector);

	this->x = components.x;
	this->y = components.y;
	this->z = components.z;
}

Vector3D::Vector3D(const DirectX::XMFLOAT3& directXFloat3) {

	this->x = directXFloat3.x;
	this->y = directXFloat3.y;
	this->z = directXFloat3.z;
}

Vector3D Vector3D::operator+(const Vector3D& right) const {
	return Vector3D(this->x + right.GetX(), this->y + right.GetY(), this->z + right.GetZ());
}

Vector3D Vector3D::operator-(const Vector3D& right) const {
	return Vector3D(this->x - right.GetX(), this->y - right.GetY(), this->z - right.GetZ());
}

float Vector3D::operator*(const Vector3D& right) const {
	return this->x * right.GetX() + this->y * right.GetY() + this->z * right.GetZ();
}

Vector3D Vector3D::operator^(const Vector3D& right) const {
	return Vector3D(this->y * right.GetZ() - this->z * right.GetY(), this->z * right.GetX() - this->x * right.GetZ(),
					this->x * right.GetY() - this->y * right.GetX());
}

Vector3D Vector3D::operator*(float scalar) const {
	return Vector3D(this->x * scalar, this->y * scalar, this->z * scalar);
}

float Vector3D::GetX() const { return this->x; }

float Vector3D::GetY() const { return this->y; }

float Vector3D::GetZ() const { return this->z; }

float Vector3D::Length() const { return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2)); }

void Vector3D::Normalize() {
	float len = this->Length();
	this->x = this->x / len;
	this->y = this->y / len;
	this->z = this->z / len;
}

std::string Vector3D::GetString() { 
	return "(" + std::to_string(this->x) + ", " + std::to_string(this->y) + ", " + std::to_string(this->z) + ")";
}