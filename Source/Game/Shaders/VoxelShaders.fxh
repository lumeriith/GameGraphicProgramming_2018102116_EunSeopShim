//--------------------------------------------------------------------------------------
// File: VoxelShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D aTextures[2] : register(t0);
SamplerState aSamplers[2] : register(s0);

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

  Summary:  Constant buffer used for world transformation, and the 
            color of the voxel
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
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader, 
            instance data included
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix Transform : INSTANCE_TRANSFORM;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Norm : NORMAL;
    float4 WorldPos : POSITION;
    float3 Color : COLOR;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VSVoxel(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    output.Position = input.Position;
    output.Position = mul(output.Position, input.Transform);
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.Color = OutputColor;
    output.Norm = normalize(mul(float4(input.Normal, 1), World).xyz);
    output.TexCoord = input.TexCoord;
    
    output.WorldPos = mul(input.Position, input.Transform);
    output.WorldPos = mul(output.WorldPos, World);
    
    if (HasNormalMap)
    {
        // Already world space
        output.Tangent = input.Tangent;
        output.Bitangent = input.Bitangent;
    }
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSVoxel(PS_INPUT input) : SV_Target
{
    float3 sampledAlbedo = aTextures[0].Sample(aSamplers[0], input.TexCoord);
    float3 normal = normalize(input.Norm);
    
    if (HasNormalMap)
    {
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);
        
        bumpMap = (bumpMap * 2.0f) - 1.0f;
        
        float3 bumpNormal = bumpMap.x * input.Tangent + bumpMap.y * input.Bitangent + bumpMap.z * normal;
        normal = normalize(bumpNormal);
    }
    
    float3 toViewDir = normalize((CameraPosition - input.WorldPos).xyz);

	
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);
		
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        float3 fromLightDir = normalize((input.WorldPos - LightPositions[i]).xyz);
	
        diffuse += max(dot(normal, -fromLightDir), 0) * LightColors[i].xyz;
		
        float3 refDir = reflect(fromLightDir, normal);
        specular += pow(max(dot(refDir, toViewDir), 0), 20) * LightColors[i].xyz;
    }

    return float4((ambient + diffuse + specular) * sampledAlbedo, 1);
}