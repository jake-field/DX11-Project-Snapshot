//Sampler States
SamplerState g_sAnisoSampler: register(s0);
SamplerState g_sShadowSampler: register(s1);

//Shadowmap tex
Texture2D g_txShadowMap: register(t0);

//Texture Data, either filled per-object or filled with default (or missing) textures in lieu of.
Texture2D g_txDiffuse: register(t1);
Texture2D g_txNormal: register(t2);
Texture2D g_txSpec: register(t3);
Texture2D g_txAO: register(t4);
Texture2D g_txEmissive: register(t9); //Temp reg.

//Structs
struct TCamera
{
	float3 vec3EyePos; float pad0;
	float3 vec3EyeLook; float pad1;
};

//Constant Buffers
cbuffer cbGlobalPerFrame : register(b0)
{
	float4x4 g_matView;
	float4x4 g_matProj;
	float4x4 g_matViewProj;
	TCamera g_tCamera;
}