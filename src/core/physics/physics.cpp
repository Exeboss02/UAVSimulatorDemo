#include "core/physics/physics.h"

inline float OverlapOnAxis(const SATData& shapeA, const SATData& shapeB, DirectX::FXMVECTOR axis)
{
    using namespace DirectX;

    // Shape A
    float A_MinDot = FLT_MAX;
    float A_MaxDot = -FLT_MAX;

    for (size_t j = 0; j < shapeA.nrOfPositions; j++)
    {
        XMVECTOR position = XMLoadFloat3(&shapeA.positionData[j]);
        float projection = XMVectorGetX(XMVector3Dot(position, axis));

        A_MinDot = std::min(A_MinDot, projection);
        A_MaxDot = std::max(A_MaxDot, projection);
    }

    // Shape B
    float B_MinDot = FLT_MAX;
    float B_MaxDot = -FLT_MAX;

    for (size_t j = 0; j < shapeB.nrOfPositions; j++)
    {
        XMVECTOR position = XMLoadFloat3(&shapeB.positionData[j]);
        float projection = XMVectorGetX(XMVector3Dot(position, axis));

        B_MinDot = std::min(B_MinDot, projection);
        B_MaxDot = std::max(B_MaxDot, projection);
    }

    float overlap = std::min(A_MaxDot, B_MaxDot) - std::max(A_MinDot, B_MinDot);

    return overlap; // positive = overlap, negative/0 = separation
}


bool SAT(const SATData& shapeA, const SATData& shapeB, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance)
{
    using namespace DirectX;

    float minOverlap = FLT_MAX;
    XMVECTOR smallestAxis = XMVectorZero();

    auto testAxis = [&](XMVECTOR axis)
    {
            axis = XMVector3Normalize(axis);
            float overlap = OverlapOnAxis(shapeA, shapeB, axis);

            if (overlap <= 0.0f) return false; // separating axis found

            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                smallestAxis = axis;
            }

            return true;
    };

    // Shape A axes
    for (int i = 0; i < shapeA.nrOfNormals; i++)
    {
        if (!testAxis(XMLoadFloat3(&shapeA.normalData[i]))) return false;
    }

    // Shape B axes
    for (int i = 0; i < shapeB.nrOfNormals; i++)
    {
        if (!testAxis(XMLoadFloat3(&shapeB.normalData[i]))) return false;
    }

    // Cross product axes
    for (int i = 0; i < shapeA.nrOfNormals; i++)
    {
        XMVECTOR axisA = XMLoadFloat3(&shapeA.normalData[i]);

        for (int j = 0; j < shapeB.nrOfNormals; j++)
        {
            XMVECTOR axisB = XMLoadFloat3(&shapeB.normalData[j]);
            XMVECTOR axis = XMVector3Cross(axisA, axisB);

            if (XMVector3LessOrEqual(XMVector3LengthSq(axis), XMVectorReplicate(1e-6f))) continue;

            if (!testAxis(axis))
            {
                return false;
            }
        }
    }

    DirectX::XMVECTOR dir = XMLoadFloat3(&shapeB.center) - XMLoadFloat3(&shapeA.center);
    if (XMVectorGetX(XMVector3Dot(dir, smallestAxis)) < 0.0f)
    {
        smallestAxis = -smallestAxis; // flip axis toward B
    }

    smallestAxis = XMVector3Normalize(smallestAxis);
    XMStoreFloat3(&resolveAxis, smallestAxis);
    resolveDistance = minOverlap;

    return true;
}

