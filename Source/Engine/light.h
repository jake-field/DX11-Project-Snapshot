#pragma once
#ifndef __LIGHT_H__
#define __LIGHT_H__

//Local Includes
#include "camera.h"

//Types
enum class TLightType
{
	PARALLEL,
	//POINT,
	//SPOT,
};

struct TLightProperties
{
	float4 vec4Ambient;
	float4 vec4Diffuse;
	float4 vec4Specular;
};

//Prototypes
class CLight final: public CCamera
{
	//Member Functions
public:
	CLight();
	~CLight();

	//TODO: Support Spot lights, then Point lights (6-pass cube map)
	//Spot lights may be easier via "decal projection" of a texture?

	//Color settings
	void SetAmbient(float4 _vec4RGBA);
	void SetDiffuse(float4 _vec4RGBA);
	void SetSpecular(float4 _vec4RGBA);
	float4 GetAmbient() const;
	float4 GetDiffuse() const;
	float4 GetSpecular() const;

	void SetDefinition(const TLightProperties& _rDefinition);
	const TLightProperties& GetDefinition() const;

	//Member Variables
protected:
	TLightProperties m_tDefinition;

};

#endif //__LIGHT_H__