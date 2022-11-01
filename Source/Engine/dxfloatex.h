#pragma once
#ifndef __DX_FLOAT_EX_H__
#define __DX_FLOAT_EX_H__

//Local Includes
#include "dxcommon.h"

//Types
struct TFloat2XM: public XMFLOAT2
{
	//Extend XMFLOAT2
	TFloat2XM()
	{
		x = 0.0f;
		y = 0.0f;
	}

	TFloat2XM(float _x, float _y)
	{
		x = _x;
		y = _y;
	}

	TFloat2XM(const TFloat2XM& _rhs)
	{
		x = _rhs.x;
		y = _rhs.y;
	}

	TFloat2XM(const XMFLOAT2& _rhs)
	{
		x = _rhs.x;
		y = _rhs.y;
	}

	bool operator==(const TFloat2XM& _rhs) const
	{
		return((floatCompare(x, _rhs.x) && floatCompare(y, _rhs.y)));
	}

	bool operator!=(const TFloat2XM& _rhs) const
	{
		return((!floatCompare(x, _rhs.x) || !floatCompare(y, _rhs.y)));
	}

	TFloat2XM operator+(const TFloat2XM& _rhs) const
	{
		return(TFloat2XM(x + _rhs.x, y + _rhs.y));
	}

	TFloat2XM operator-(const TFloat2XM& _rhs) const
	{
		return(TFloat2XM(x - _rhs.x, y - _rhs.y));
	}

	TFloat2XM operator*(const float& _rhs) const
	{
		return(TFloat2XM(x * _rhs, y * _rhs));
	}

	TFloat2XM operator/(const float& _rhs) const
	{
		return(TFloat2XM(x / _rhs, y / _rhs));
	}

	TFloat2XM& operator+=(const TFloat2XM& _rhs)
	{
		x += _rhs.x;
		y += _rhs.y;
		return(*this);
	}

	TFloat2XM& operator-=(const TFloat2XM& _rhs)
	{
		x -= _rhs.x;
		y -= _rhs.y;
		return(*this);
	}

	TFloat2XM& operator*=(const float& _rhs)
	{
		x *= _rhs;
		y *= _rhs;
		return(*this);
	}

	TFloat2XM& operator/=(const float& _rhs)
	{
		x /= _rhs;
		y /= _rhs;
		return(*this);
	}
};

struct TFloat3XM: public XMFLOAT3
{
	//Extend XMFLOAT3
	TFloat3XM()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	TFloat3XM(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	TFloat3XM(const TFloat3XM& _rhs)
	{
		x = _rhs.x;
		y = _rhs.y;
		z = _rhs.z;
	}

	TFloat3XM(const XMFLOAT3& _rhs)
	{
		x = _rhs.x;
		y = _rhs.y;
		z = _rhs.z;
	}

	bool operator==(const TFloat3XM& _rhs) const
	{
		return((floatCompare(x, _rhs.x) && floatCompare(y, _rhs.y) && floatCompare(z, _rhs.z)));
	}

	bool operator!=(const TFloat3XM& _rhs) const
	{
		return((!floatCompare(x, _rhs.x) || !floatCompare(y, _rhs.y) || !floatCompare(z, _rhs.z)));
	}

	//used for accessing like an array
	float& operator[](int i)
	{
		//assume SxS in mem
		//TODO: this will probably break and cause grief
		return(reinterpret_cast<float*>(this)[i]);
	}

	TFloat3XM operator+(const TFloat3XM& _rhs) const
	{
		return(TFloat3XM(x + _rhs.x, y + _rhs.y, z + _rhs.z));
	}

	TFloat3XM operator-(const TFloat3XM& _rhs) const
	{
		return(TFloat3XM(x - _rhs.x, y - _rhs.y, z - _rhs.z));
	}

	TFloat3XM operator*(const TFloat3XM& _rhs) const
	{
		return(TFloat3XM(x * _rhs.x, y * _rhs.y, z * _rhs.z));
	}

	TFloat3XM operator*(const float& _rhs) const
	{
		return(TFloat3XM(x * _rhs, y * _rhs, z * _rhs));
	}

	TFloat3XM operator/(const float& _rhs) const
	{
		return(TFloat3XM(x / _rhs, y / _rhs, z / _rhs));
	}

	TFloat3XM& operator+=(const TFloat3XM& _rhs)
	{
		x += _rhs.x;
		y += _rhs.y;
		z += _rhs.z;
		return(*this);
	}

	TFloat3XM& operator-=(const TFloat3XM& _rhs)
	{
		x -= _rhs.x;
		y -= _rhs.y;
		z -= _rhs.z;
		return(*this);
	}

	TFloat3XM& operator*=(const float& _rhs)
	{
		x *= _rhs;
		y *= _rhs;
		z *= _rhs;
		return(*this);
	}

	TFloat3XM& operator/=(const float& _rhs)
	{
		x /= _rhs;
		y /= _rhs;
		z /= _rhs;
		return(*this);
	}

	float Mag() const
	{
		return(sqrtf((x * x) + (y * y) + (z * z)));
	}

	float Dot(const TFloat3XM& _rhs) const
	{
		return((x * _rhs.x) + (y * _rhs.y) + (z * _rhs.z));
	}

	TFloat3XM Normalize() const
	{
		TFloat3XM vec3Result = *this;
		float fMag = vec3Result.Mag();
		if(fMag != 0.0f) vec3Result *= (1.0f / fMag);
		return(vec3Result);
	}
};

struct TFloat4x4XM: public XMFLOAT4X4
{
	TFloat4x4XM()
	{
		ZeroMemory(this, sizeof(TFloat4x4XM));
	}

	TFloat4x4XM(const XMFLOAT4X4& _rhs)
	{
		memcpy_s(this, sizeof(XMFLOAT4X4), &_rhs, sizeof(XMFLOAT4X4));
	}

	XMFLOAT4X4 Transpose() const
	{
		return(XMFLOAT4X4(_11, _21, _31, _41,
						  _12, _22, _32, _42,
						  _13, _23, _33, _43,
						  _14, _24, _34, _44));
	}

	static XMFLOAT4X4 Identity()
	{
		return(XMFLOAT4X4(	1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f));
	}
};

#endif //__DX_FLOAT_EX_H__
