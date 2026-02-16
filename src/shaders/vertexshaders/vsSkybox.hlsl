struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 direction : DIRECTION;
};

cbuffer ViewProjMatrixBuffer : register(b0)
{
    row_major float4x4 viewProjectionMatrix;
    float4 cameraPos;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    output.position = mul(float4(input.position + cameraPos.xyz, 1.0f), viewProjectionMatrix).xyww;
    output.direction = input.position;
    
    return output;
}