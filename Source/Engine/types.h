#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__

//TODO: Namespaces
//TODO: Better file name

//Local Includes
#include "dxfloatex.h"

//Type definitions
#ifdef __DXCOMMON_H__
typedef TFloat2XM float2; //Extends XMFLOAT2
typedef TFloat3XM float3; //Extends XMFLOAT3
typedef XMFLOAT4 float4;
typedef TFloat4x4XM float4x4; //Extends XMFLOAT4X4
#else
struct float2
{
	float x, y;

	float2()
		: x(0.0f)
		, y(0.0f)
	{
	}

	float2(float _x, float _y)
		: x(_x)
		, y(_y)
	{
	}
};

struct float3
{
	float x, y, z;

	float3()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
	{
	}

	float3(float _x, float _y, float _z)
		: x(_x)
		, y(_y)
		, z(_z)
	{
	}
};

struct float4
{
	float x, y, z, w;

	float4()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, w(0.0f)
	{
	}

	float4(float _x, float _y, float _z, float _w)
		: x(_x)
		, y(_y)
		, z(_z)
		, w(_w)
	{
	}
};
#endif 

#endif //__TYPES_H__