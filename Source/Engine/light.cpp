//This Include
#include "light.h"

//Implementation
CLight::CLight()
{
	//Constructor
	ZeroMemory(&m_tDefinition, sizeof(TLightProperties));
}

CLight::~CLight()
{
	//Destructor
}

void
CLight::SetAmbient(float4 _vec4RGBA)
{
	m_tDefinition.vec4Ambient = _vec4RGBA;
}

void
CLight::SetDiffuse(float4 _vec4RGBA)
{
	m_tDefinition.vec4Diffuse = _vec4RGBA;
}

void
CLight::SetSpecular(float4 _vec4RGBA)
{
	m_tDefinition.vec4Specular = _vec4RGBA;
}

float4
CLight::GetAmbient() const
{
	return(m_tDefinition.vec4Ambient);
}

float4
CLight::GetDiffuse() const
{
	return(m_tDefinition.vec4Diffuse);
}

float4
CLight::GetSpecular() const
{
	return(m_tDefinition.vec4Specular);
}

void
CLight::SetDefinition(const TLightProperties& _rDefinition)
{
	m_tDefinition = _rDefinition;
}

const TLightProperties&
CLight::GetDefinition() const
{
	return(m_tDefinition);
}