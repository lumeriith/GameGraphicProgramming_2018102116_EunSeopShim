//--------------------------------------------------------------------------------------
// File: ShadowShaders.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer cbShadowMatrix : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
    bool isVoxel;
}

struct VS_SHADOW_INPUT
{
	float4 Position : POSITION;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
};


struct PS_SHADOW_INPUT
{
    float4 Position : SV_POSITION;
	float4 DepthPosition : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_SHADOW_INPUT VSShadow(VS_SHADOW_INPUT input)
{
    PS_SHADOW_INPUT output = (PS_SHADOW_INPUT) 0;
    output.Position = input.Position;
	
	if (isVoxel)
        output.Position = mul(output.Position, input.mTransform);
	
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
	
    output.DepthPosition = output.Position;
	
    return output;
};


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSShadow(PS_SHADOW_INPUT input) : SV_Target
{
    float depthVal = input.DepthPosition.z / input.DepthPosition.w;
    return float4(depthVal, depthVal, depthVal, 1.0f);
};