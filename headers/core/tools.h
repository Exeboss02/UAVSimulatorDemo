#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include <d3d11.h>
#include <string>
#include <Windows.h>
#include <math.h>

class Timer
{
public:
	float currentTime = 0;
	float startTime = 0;

	Timer() = default;
	~Timer() = default;

	void Initialize(float startTime);
	void Tick(float deltaTime);
	bool TimeIsUp();
	void Reset();
	bool IsReady();
};

DirectX::XMFLOAT3 FLOAT3ADD(DirectX::XMFLOAT3 vector, DirectX::XMFLOAT3 addVector);
DirectX::XMFLOAT3 FLOAT3SUB(DirectX::XMFLOAT3 vector, DirectX::XMFLOAT3 subVector);
DirectX::XMFLOAT3 FLOAT3MULT1(DirectX::XMFLOAT3 vector, float scalar);
DirectX::XMFLOAT3 FLOAT3MULTFLOAT3(DirectX::XMFLOAT3 vector, DirectX::XMFLOAT3 otherVector);
DirectX::XMFLOAT3 FLOAT3LERP(DirectX::XMFLOAT3 minVector, DirectX::XMFLOAT3 maxVector, float alpha);
float LERP(float minFloat, float maxFloat, float alpha);

std::wstring ConvertToWideString(const std::string& shortString);
void PrintMatrix(const DirectX::XMFLOAT4X4& matrix);
void PrintFloat3(const DirectX::XMFLOAT3 vector, std::string label);
float GetLengthOfFLOAT3(const DirectX::XMFLOAT3& vector);
int RandomInt(int lowLimit, int highLimit);
std::string GetExecutablePath();
DirectX::XMFLOAT3 FLOAT3PROJECT(DirectX::XMFLOAT3 sourceVectorm, DirectX::XMFLOAT3 targetVector);
DirectX::XMFLOAT3 FLOAT3NORMALIZE(DirectX::XMFLOAT3 vector);
