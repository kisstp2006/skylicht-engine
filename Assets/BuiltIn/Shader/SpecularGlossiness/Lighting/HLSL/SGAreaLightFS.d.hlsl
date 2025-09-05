Texture2D uTexPosition : register(t0);
SamplerState uTexPositionSampler : register(s0);

Texture2D uTexNormal : register(t1);
SamplerState uTexNormalSampler : register(s1);

Texture2D uTexData : register(t2);
SamplerState uTexDataSampler : register(s2);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex0 : TEXCOORD0;
};

cbuffer cbPerFrame
{
	float4 uCameraPosition;
	float4 uLightPosition;
	float3 uLightDirX;
	float3 uLightDirY;
	float2 uLightSize;
	float4 uLightColor;
};

#include "../../../Light/HLSL/LibAreaLight.hlsl"

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 position = uTexPosition.Sample(uTexPositionSampler, input.tex0).xyz;
	float3 normal = uTexNormal.Sample(uTexNormalSampler, input.tex0).xyz;

	float3 lightColor = arealight(
		position, 
		normal,
		uCameraPosition.xyz, 
		uLightColor, 
		uLightPosition.xyz, 
		uLightDirX, 
		uLightDirY,
		uLightSize);
	
	return float4(lightColor, 1.0);
}