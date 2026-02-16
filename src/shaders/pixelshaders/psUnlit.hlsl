struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cameraPosition : CAMERA_POSITION;
};

cbuffer MaterialBuffer : register(b1)
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float shininess;
    int textureSlots;
};

Texture2D diffuseTexture : register(t1);

SamplerState mainSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // so idk if "Sample()" or "if" is most expensive, I just guessed.
    
    float4 textureColor = float4(1, 1, 1, 1);
    if (textureSlots > 0)
    {
        textureColor = diffuseTexture.Sample(mainSampler, float2(input.uv.x, 1 - input.uv.y));
    }
        
    return textureColor * diffuse;
}