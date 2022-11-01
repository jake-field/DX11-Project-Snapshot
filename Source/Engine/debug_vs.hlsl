//Global shader data include
#include "global.hlsli"

//Per obj world matrix. The view/proj are loaded via the global shader
cbuffer cbPerObject: register(b1)
{
	float4x4 matWorld;
};

//Basic position data
float4 main(float4 pos : POSITION): SV_POSITION
{
	float4x4 matWVP = mul(matWorld, mul(g_matView, g_matProj));
	return(mul(pos, matWVP));
}
