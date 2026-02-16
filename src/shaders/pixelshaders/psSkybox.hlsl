struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 direction : DIRECTION;
};

TextureCube cubeMap : register(t1);

SamplerState mainSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // swizzle because the skybox was rotated
    return cubeMap.Sample(mainSampler, normalize(input.direction.xzy));
}