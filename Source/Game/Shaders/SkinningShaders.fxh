//--------------------------------------------------------------------------------------
// File: SkinningShaders.fx
//
// Copyright (c) Microsoft Corporation.
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static const unsigned int MAX_NUM_BONES = 256u;
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation
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

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbSkinning

  Summary:  Constant buffer used for skinning
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbSkinning : register(b4)
{
    row_major matrix BoneTransforms[MAX_NUM_BONES];
};

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_PHONG_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    uint4 BoneIndices : BONEINDICES;
    float4 BoneWeights : BONEWEIGHTS;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_PHONG_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
    float4 WorldPos : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSPhong(VS_PHONG_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT) 0;
    
    // Calculate Skin Matrix
    matrix skin = BoneTransforms[input.BoneIndices.x] * input.BoneWeights.x;
    skin += BoneTransforms[input.BoneIndices.y] * input.BoneWeights.y;
    skin += BoneTransforms[input.BoneIndices.z] * input.BoneWeights.z;
    skin += BoneTransforms[input.BoneIndices.w] * input.BoneWeights.w;
    
    // Calculate Position
    output.Pos = input.Position;
    output.Pos = mul(output.Pos, skin);
    output.Pos = mul(output.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    
    // Calculate Normal
    float4x4 skinNoTranslation = skin;
    skinNoTranslation[0].w = 0;
    skinNoTranslation[1].w = 0;
    skinNoTranslation[2].w = 0;
    skinNoTranslation[3].x = 0;
    skinNoTranslation[3].y = 0;
    skinNoTranslation[3].z = 0;
    skinNoTranslation[3].w = 1;

    float4 normal = float4(input.Normal, 1);

    normal = mul(normal, skinNoTranslation);
    normal = mul(normal, World);
    output.Norm = normalize(normal.xyz);
    
    // Others...
    output.Tex = input.TexCoord;
    
    output.WorldPos = mul(input.Position, skin);
    output.WorldPos = mul(output.WorldPos, World);

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSPhong(PS_PHONG_INPUT input) : SV_Target
{
    float3 toViewDir = normalize((CameraPosition - input.WorldPos).xyz);
    float3 normal = normalize(input.Norm);
	
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

    return float4(saturate(ambient + diffuse + specular), 1) * txDiffuse.Sample(samLinear, input.Tex);
}
