#include "core/tools.h"

DirectX::XMFLOAT3 FLOAT3ADD(DirectX::XMFLOAT3 vector, DirectX::XMFLOAT3 addVector)
{
    DirectX::XMFLOAT3 returnVector;

    returnVector.x = vector.x + addVector.x;
    returnVector.y = vector.y + addVector.y;
    returnVector.z = vector.z + addVector.z;

    return returnVector;
}

DirectX::XMFLOAT3 FLOAT3SUB(DirectX::XMFLOAT3 vector, DirectX::XMFLOAT3 subVector)
{
    DirectX::XMFLOAT3 returnVector;

    returnVector.x = vector.x - subVector.x;
    returnVector.y = vector.y - subVector.y;
    returnVector.z = vector.z - subVector.z;

    return returnVector;
}

DirectX::XMFLOAT3 FLOAT3MULT1(DirectX::XMFLOAT3 vector, float scalar)
{
    return DirectX::XMFLOAT3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
}

DirectX::XMFLOAT3 FLOAT3MULTFLOAT3(DirectX::XMFLOAT3 vector, DirectX::XMFLOAT3 otherVector)
{
    DirectX::XMFLOAT3 newVector = DirectX::XMFLOAT3
    (
        vector.x * otherVector.x,
        vector.y * otherVector.y,
        vector.z * otherVector.z
    );

    return newVector;
}

DirectX::XMFLOAT3 FLOAT3LERP(DirectX::XMFLOAT3 minVector, DirectX::XMFLOAT3 maxVector, float alpha)
{
    using namespace DirectX;

    XMVECTOR min = DirectX::XMLoadFloat3(&minVector);
    XMVECTOR max = DirectX::XMLoadFloat3(&maxVector);
    XMFLOAT3 result;
    
    XMStoreFloat3(&result, XMVectorLerp(min, max, alpha));

    return result;
}

std::wstring ConvertToWideString(const std::string& shortString)
{
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, shortString.c_str(), -1, nullptr, 0);
    std::wstring wideStr(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, shortString.c_str(), -1, &wideStr[0], sizeNeeded);

    return wideStr;
}

void PrintMatrix(const DirectX::XMFLOAT4X4& matrix)
{
    std::cout << "-----------::MATRIX::----------" << std::endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix.m[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-----------::MATRIX::----------" << std::endl;
}

void PrintFloat3(const DirectX::XMFLOAT3 vector, std::string label)
{
    std::cout << label << ": " << vector.x << ", " << vector.y << ", " << vector.z << std::endl;
}

float GetLengthOfFLOAT3(const DirectX::XMFLOAT3& vector)
{
    DirectX::XMVECTOR tempVector = XMLoadFloat3(&vector);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(tempVector);

    float result;
    DirectX::XMStoreFloat(&result, length);

    return result;
}

int RandomInt(int lowLimit, int highLimit)
{
    return lowLimit + rand() % (highLimit - lowLimit + 1);
}

void Timer::Initialize(float startTime)
{
    this->startTime = startTime;
    this->currentTime = startTime;
}

void Timer::Tick(float deltaTime)
{
    if(this->currentTime >= 0) this->currentTime -= deltaTime;
}

bool Timer::TimeIsUp()
{
    if (this->currentTime <= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Timer::Reset()
{
    this->currentTime = this->startTime;
}

bool Timer::IsReady()
{
    return (this->currentTime == this->startTime);
}

std::string GetExecutablePath()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH); // gets full path to exe
    std::string fullPath(buffer);
    size_t lastSlash = fullPath.find_last_of("\\/");

    return fullPath.substr(0, lastSlash); // folder containing exe
}

DirectX::XMFLOAT3 FLOAT3PROJECT(DirectX::XMFLOAT3 sourceVector, DirectX::XMFLOAT3 targetVector)
{
    using namespace DirectX;

    if (targetVector.x == 0 && targetVector.y == 0 && targetVector.z == 0)
    {
        return XMFLOAT3(0, 0, 0);
    }

    XMVECTOR v1 = XMLoadFloat3(&sourceVector);
    XMVECTOR v2 = XMLoadFloat3(&targetVector);

    XMVECTOR projection = XMVectorScale(v2, XMVectorGetX(XMVector3Dot(v1, v2)) / XMVectorGetX(XMVector3Dot(v2, v2)));
    XMFLOAT3 projectedVector = {};
    XMStoreFloat3(&projectedVector, projection);

    return projectedVector;
}

DirectX::XMFLOAT3 FLOAT3NORMALIZE(DirectX::XMFLOAT3 vector)
{
    using namespace DirectX;

    XMVECTOR normalizedVector = XMLoadFloat3(&vector);
    normalizedVector = XMVector3Normalize(normalizedVector);
    XMStoreFloat3(&vector, normalizedVector);

    return vector;
}
