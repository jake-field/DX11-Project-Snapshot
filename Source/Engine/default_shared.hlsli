//Includes
#include "global.hlsli" //Global shared buffers/states/data

//Structs
struct LightDesc //Light struct for cbuffer
{
	float3 pos; float fPad0;
	float3 dir; float fPad1;
	float4 ambient;
	float4 diffuse;
	float4 spec;
};

struct SurfaceInfo
{
	float3 pos;
	float4 posLightSpace;
	float3 normal;
	float4 diffuse;
	float spec;
	float aoValue; //ambient occlusion (using value from texture or 1.0f for no-change)
};

struct VS_IN
{
	float3 vPosL : POSITION0;
	float3 vNormalL : NORMAL0;
	float3 vTangentL : TANGENT0; //Not used currently
	//float3 vBitangentL
	float2 vTexCoord : TEXCOORD0;
};

struct RIGGED_VS_IN: VS_IN
{
	int4 vBoneIds : BONE_IDS;
	float4 vBoneWeights : BONE_WEIGHTS;
};

//Inherits from VS_IN, this is just the instance information tacked on
struct VS_IN_INSTANCED: VS_IN
{
	float3 vInstancePos		: I_POSITION;
	float3 vInstanceScale	: I_SCALE;
	float4 vInstanceRot		: I_ROTATION; //quaternion
};

struct VS_OUT
{
	float4 vPosH : SV_POSITION;
	float4 vPosW : POSITION0;
	float4 vPosLight : POSITION1;
	float4 vNormalW : NORMAL0;
	//float3 vTangentW : TANGENT0;
	//float3 vTangentLight : TANGENT1;
	//float3 vTangentView : TANGENT2;
	float2 vTexCoord : TEXCOORD0;
};

//Constant Buffers
//register(b0) = global.hlsli
cbuffer cbPerFrame: register(b1)
{
	float4x4 matSunVP;
	LightDesc g_tLight;
};

cbuffer cbPerObject: register(b2)
{
	float4x4 matWorld;
	bool bRenderUnlit; bool3 pack1; //packs to equiv. float1
	float3 packing; //brings the bools up to float4 total
};

float3 RotateVectorByQuaternion(float4 Q, float3 V)
{
	return(V + 2.0f * cross(Q.xyz, cross(Q.xyz, V) + Q.w * V));
}