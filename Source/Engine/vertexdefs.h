#pragma once
#ifndef __VERTEX_DEFINES_H__
#define __VERTEX_DEFINES_H__

//Local Includes
#include "types.h"

//Types
struct TVertexTexNorm
{
	//Variables
	float3 pos;
	float3 normal;
	float3 tangent;
	float2 texcoord;

	//Functions
	TVertexTexNorm()
	{
		ZeroMemory(this, sizeof(TVertexTexNorm));
	}

	TVertexTexNorm(float3 _vec3Pos, float3 _vec3Norm, float2 _vec2TexCoord)
		: pos(_vec3Pos)
		, normal(_vec3Norm)
		, texcoord(_vec2TexCoord)
	{
		//Constructor
	}
};

#endif //__VERTEX_DEFINES_H__