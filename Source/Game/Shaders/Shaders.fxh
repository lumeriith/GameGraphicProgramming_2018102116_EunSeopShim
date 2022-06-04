//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (1)
#define NEAR_PLANE (0.01f)
#define FAR_PLANE (1000.0f)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D aTextures[2] : register(t0);
SamplerState aSamplers[2] : register(s0);

Texture2D shadowMapTexture : register(t2);
SamplerState shadowMapSampler : register(s2);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
    bool HasNormalMap;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PointLight
{
    float4 Position;
    float4 Color;
    matrix View;
    matrix Projection;
    float4 AttenuationDistance;
};

cbuffer cbLights : register(b3)
{
    PointLight PointLights[NUM_LIGHTS];
};

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_PHONG_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_PHONG_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 LightViewPosition : TEXCOORD1;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_LIGHT_CUBE_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_LIGHT_CUBE_INPUT
{
    float4 Position : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSEnvironmentMap(VS_PHONG_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT) 0;
	
    output.Position = input.Position;
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = mul(float4(input.Normal, 0.0f), World).xyz;
    output.TexCoord = input.TexCoord;
	
    output.WorldPosition = mul(input.Position, World);
	
    if (HasNormalMap)
    {
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }
    
    output.LightViewPosition = input.Position;
    output.LightViewPosition = mul(output.LightViewPosition, World);
    output.LightViewPosition = mul(output.LightViewPosition, PointLights[0].View);
    output.LightViewPosition = mul(output.LightViewPosition, PointLights[0].Projection);
    
    return output;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE))) / FAR_PLANE;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSEnvironmentMap(PS_PHONG_INPUT input) : SV_Target
{
    float3 toViewDir = normalize(CameraPosition.xyz - input.WorldPosition);
    float3 normal = normalize(input.Normal);
	
    if (HasNormalMap)
    {
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);
        
        bumpMap = (bumpMap * 2.0f) - 1.0f;
        
        float3 bumpNormal = bumpMap.x * input.Tangent + bumpMap.y * input.Bitangent + bumpMap.z * normal;
        normal = normalize(bumpNormal);
    }
	
    float4 albedo = aTextures[0].Sample(aSamplers[0], input.TexCoord);
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);
		
    float2 depthTexCoord =
    {
        input.LightViewPosition.x / input.LightViewPosition.w / 2.0f + 0.5f,
        -input.LightViewPosition.y / input.LightViewPosition.w / 2.0f + 0.5f
    };
    
    float closestDepth = shadowMapTexture.Sample(shadowMapSampler, depthTexCoord).r;
    closestDepth = LinearizeDepth(closestDepth);
    
    float currentDepth = input.LightViewPosition.z / input.LightViewPosition.w;
    currentDepth = LinearizeDepth(currentDepth);

    if (currentDepth > closestDepth + 0.001f) // Shadow bias
    {
        return float4(ambient * albedo.rgb, 1);
    }
    
    float shininess = 20;
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        float attEpsilon = 0.000001f;
        float sqrDist = dot(
            input.WorldPosition - PointLights[i].Position.xyz,
            input.WorldPosition - PointLights[i].Position.xyz
        );
        float attFactor = PointLights[i].AttenuationDistance.z / (sqrDist + attEpsilon);
        float4 attLightColor = PointLights[i].Color * attFactor;
        
        float3 fromLightDir = normalize(input.WorldPosition - PointLights[i].Position.xyz);
	
        diffuse += max(dot(normal, -fromLightDir), 0) * attLightColor.xyz;
		
        float3 refDir = reflect(fromLightDir, normal);
        specular += pow(max(dot(refDir, toViewDir), 0), shininess) * attLightColor.xyz;
    }

    return float4(ambient + diffuse + specular, 1) * albedo;
}

// Placeholder
PS_PHONG_INPUT VS(VS_PHONG_INPUT input)
{
    return (PS_PHONG_INPUT) 0;
}

float4 PS(PS_PHONG_INPUT input) : SV_Target
{
    return float4(1, 1, 1, 1);
}