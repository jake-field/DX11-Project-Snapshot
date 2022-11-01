#pragma once
#ifndef __SHADER_GLOBALS_H__
#define __SHADER_GLOBALS_H__

//Local Includes
#include "types.h"
#include "dxcommon.h"

//global.hlsli
class CTexture;
namespace ShaderGlobals
{
	//Occupied shader registers
	enum ERegisters
	{
		//Constant Buffer registers
		CB_GlobalCBuffer = 0,
		CB_UsableStartSlot, //First usable unoccupied start slot

		//Sampler state registers
		SAMP_ANISO = 0,
		SAMP_SHADOW,
		SAMP_UsableStartSlot, //First usable unoccupied start slot

		//Texture registers
		TX_SHADOWMAP = 0, //This may change/widen if CSM is implemented
		TX_DIFFUSE,
		TX_NORMAL,
		TX_SPECULAR,
		TX_AO,
		TX_EMISSIVE,
		TX_UsableStartSlot, //First usable unoccupied start slot
	};

	//CB0
	static struct TCBufferGlobal
	{
		//Variables
		float4x4 matView;
		float4x4 matProj;
		float4x4 matViewProj;

		struct TCameraValues
		{
			float3 vec3EyePos; float pad0; //Active Camera Pos
			float3 vec3EyeLook; float pad1; //Active Camera Look
		} tCamera;

		//Functions
		TCBufferGlobal()
		{
			matView, matProj, matViewProj = float4x4::Identity();
			ZeroMemory(&tCamera, sizeof(TCameraValues));
		}

	} gGlobalCBuffer;
}

#endif //__SHADER_GLOBALS_H__