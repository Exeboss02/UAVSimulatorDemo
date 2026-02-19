#include "light.hlsl"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cameraPosition : CAMERA_POSITION;
};

cbuffer SpotlightCountBuffer : register(b0)
{
    int spotlightCount;
};

cbuffer PointlightCountBuffer : register(b2)
{
    int pointLightCount;
};

cbuffer MaterialBuffer : register(b1)
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float shininess;
    int textureSlots;
};

struct Spotlight
{
    float3 position;
    float3 direction;

    float4 color;
    float intensity;

    float spotCosAngle;
    float4x4 vpMatrix;
};
struct PointLight
{
    float3 position;
    float4 color;
    float intensity;

    float4x4 vpMatrix[6];
};

Texture2D diffuseTexture : register(t1);
Texture2D ambientTexture : register(t2);
Texture2D specularTexture : register(t3);
Texture2D normalTexture : register(t4);

StructuredBuffer<Spotlight> spotlightBuffer : register(t0);
Texture2DArray<unorm float> shadowMaps : register(t5);

StructuredBuffer<PointLight> pointLightBuffer : register(t6);
TextureCubeArray<unorm float> pointLightShadowMaps : register(t7);

SamplerState mainSampler : register(s0);
SamplerState shadowSampler : register(s1);

float3 ComputeNormal(float3 fragmentpos, float2 fragmentUV, float3 camPos, float3 fragmentNormal, float3 normalSample);

float4 main(PixelShaderInput input) : SV_TARGET
{    
    const float bias = 0.001f;
    float3 normal = normalize(input.normal);
    
    float4 ambientColor = ambient;
    float4 diffuseColor = 0;
    float4 specularColor = 0;
    float3 camToPixel = input.worldPosition.xyz - input.cameraPosition;
    
    uint numLights, stride;
    spotlightBuffer.GetDimensions(numLights, stride);
    
    // Move to cpu
    float2 inputUV = float2(input.uv.x, 1 - input.uv.y);
    float hasNormal = (textureSlots & 8) != 0;
    if (hasNormal)
    {
        float4 normalSample = normalTexture.Sample(mainSampler, inputUV);
        normal = ComputeNormal(input.worldPosition.xyz, inputUV, input.cameraPosition, input.normal, normalSample.xyz);
    }
        
    // Seems structured buffer might be made larger than the number of lights, as such we do need to use ugly constant buffer
    for (int i = 0; i < spotlightCount; i++)
    {      
        Spotlight lightdata = spotlightBuffer[i];
                
        float4 lightClip = mul(float4(input.worldPosition.xyz, 1), lightdata.vpMatrix);
        float3 ndc = lightClip.xyz / lightClip.w;
        
        float2 uv = float2(ndc.x * 0.5f + 0.5f, ndc.y * -0.5f + 0.5f);
        
        float sceneDepth = ndc.z;
        float mapDepth = shadowMaps.SampleLevel(shadowSampler, float3(uv, i), 0.f).r;
        
        bool islit = (mapDepth + bias) >= sceneDepth;
        
        
        float3 LightToHit = input.worldPosition.xyz - lightdata.position;
        float3 lightDir = normalize(LightToHit);
        float lightCosAngle = dot(lightDir.xyz, normalize(lightdata.direction));
        
        if (lightCosAngle > lightdata.spotCosAngle && islit)
        {
            float3 L = normalize(-LightToHit);
            float3 V = normalize(-camToPixel);
            float3 halfwayVector = normalize(L + V);
            
            float distSq = max(dot(LightToHit, LightToHit), 1e-6f);
          
            float intensity = (1.0f / distSq) * lightdata.intensity;

            float nDotL = dot(normal, L);
            float nDotHalf = dot(normal, halfwayVector);
            float4 lighting = lit(nDotL, nDotHalf, shininess);

            diffuseColor += lighting.y * lightdata.color * intensity;
            specularColor += lighting.z * lightdata.color * intensity;
        }
    }
    for (i = 0; i < pointLightCount; i++)
    {
        PointLight lightdata = pointLightBuffer[i];
        // Vector from light to the fragment
        float3 LightToHit = input.worldPosition.xyz - lightdata.position;
        float3 lightDir = normalize(LightToHit);

        // Direction used to sample the cubemap
        float3 sampleDir = normalize(LightToHit);

        // Determine which cubemap face the direction maps to so we can use
        // the corresponding view-projection matrix that was used when rendering
        // that face. Cube face ordering: +X, -X, +Y, -Y, +Z, -Z -> 0..5
        int faceIndex = 0;
        float3 absDir = abs(sampleDir);
        if (absDir.x >= absDir.y && absDir.x >= absDir.z)
        {
            faceIndex = sampleDir.x > 0 ? 0 : 1;
        }
        else if (absDir.y >= absDir.x && absDir.y >= absDir.z)
        {
            faceIndex = sampleDir.y > 0 ? 2 : 3;
        }
        else
        {
            faceIndex = sampleDir.z > 0 ? 4 : 5;
        }

        // Project the world position with the same view-projection matrix
        // that was used to render the corresponding cubemap face and compare
        // the resulting depth (z/w) with the sampled depth from the cubemap.
        float4 lightClip = mul(float4(input.worldPosition.xyz, 1), lightdata.vpMatrix[faceIndex]);
        float sceneDepth = lightClip.z / lightClip.w;        
        
        float mapDepth = pointLightShadowMaps.SampleLevel(shadowSampler, float4(sampleDir, i), 0).r;
        bool islit = (mapDepth + bias) >= sceneDepth;

        if (islit)
        {
            float3 L = normalize(-LightToHit);
            float3 V = normalize(-camToPixel);
            float3 halfwayVector = normalize(L + V);
            
            float distSq = max(dot(LightToHit, LightToHit), 1e-6f);
          
            float intensity = (1.0f / distSq) * lightdata.intensity;

            float nDotL = dot(normal, L);
            float nDotHalf = dot(normal, halfwayVector);
            float4 lighting = lit(nDotL, nDotHalf, shininess);

            diffuseColor += lighting.y * lightdata.color * intensity;
            specularColor += lighting.z * lightdata.color * intensity;
        }
    }
    
    // Read textures
    // It's done in a way to avoid if-statements
    
    float2 uv = float2(input.uv.x, 1 - input.uv.y);
    
    float hasDiffuse = (textureSlots & 1) != 0;
    float hasAmbient = (textureSlots & 2) != 0;
    float hasSpecular = (textureSlots & 4) != 0;

    float4 diffuseSample = diffuseTexture.Sample(mainSampler, uv);
    float4 ambientSample = ambientTexture.Sample(mainSampler, uv);
    float4 specularSample = specularTexture.Sample(mainSampler, uv);

    float4 diffuseTextureColor =
    lerp(float4(1, 1, 1, 1), diffuseSample, hasDiffuse);

    // Ambient falls back to diffuse if there is no dedicated ambient texture
    float4 ambientTextureColor =
    lerp(diffuseTextureColor, ambientSample, hasAmbient); 

    float4 specularTextureColor =
    lerp(float4(1, 1, 1, 1), specularSample, hasSpecular);

    float4 color = diffuseTextureColor * diffuseColor + ambientTextureColor * ambientColor + specularTextureColor * specularColor;
    
    return color;
}

float3 ComputeNormal(float3 fragmentpos, float2 fragmentUV, float3 camPos, float3 fragmentNormal, float3 normalSample)
{
    // compute tangent and Bitangent
    float3 dPdx = ddx(fragmentpos);
    float3 dPdy = ddy(fragmentpos);
    float2 dUVdx = ddx(fragmentUV);
    float2 dUVdy = ddy(fragmentUV);

    float3 tangent = normalize(dUVdy.y * dPdx - dUVdx.y * dPdy);
    tangent = normalize(tangent - fragmentNormal.xyz * dot(fragmentNormal.xyz, tangent));
        
    float3 bitangent = cross(fragmentNormal.xyz, tangent);
        
        // Convert view direction to tangent space
    float3 viewDir = normalize(fragmentpos.xyz - camPos);
    float3x3 TBN = float3x3(tangent, bitangent, fragmentNormal.xyz);
    float3 viewDirTangent = normalize(mul(TBN, viewDir));
    
    float3 worldNormal = normalize(
            normalSample.x * tangent +
            normalSample.y * bitangent +
            normalSample.z * fragmentNormal
        );
    return worldNormal;
}