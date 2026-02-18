struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    
    float4 world0 : WORLD_MATRIX0;
    float4 world1 : WORLD_MATRIX1;
    float4 world2 : WORLD_MATRIX2;
    float4 world3 : WORLD_MATRIX3;

    float4 worldIT0 : INVERSED_TRANSPOSED_WORLD_MATRIX0;
    float4 worldIT1 : INVERSED_TRANSPOSED_WORLD_MATRIX1;
    float4 worldIT2 : INVERSED_TRANSPOSED_WORLD_MATRIX2;
    float4 worldIT3 : INVERSED_TRANSPOSED_WORLD_MATRIX3;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cameraPosition : CAMERA_POSITION;
};

cbuffer ViewProjMatrixBuffer : register(b0)
{
    row_major float4x4 viewProjectionMatrix;
    float4 cameraPos;
};

VertexShaderOutput main(VertexShaderInput input)
{    
    float4x4 worldMatrix = float4x4(
    input.world0,
    input.world1,
    input.world2,
    input.world3);

    float4x4 worldMatrixInversedTransposed = float4x4(
    input.worldIT0,
    input.worldIT1,
    input.worldIT2,
    input.worldIT3);
    
    
    
    VertexShaderOutput output;
    
    float4 pos = float4(input.position, 1.0f);
    output.position = mul(mul(pos, worldMatrix), viewProjectionMatrix);
    output.worldPosition = mul(pos, worldMatrix);
    output.normal = mul(input.normal, (float3x3) worldMatrixInversedTransposed); 
    output.uv = input.uv;
    output.cameraPosition = cameraPos.xyz;
    
    return output;
}