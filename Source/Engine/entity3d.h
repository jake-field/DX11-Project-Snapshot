#pragma once
#ifndef __ENTITY_3D_H__
#define __ENTITY_3D_H__

//Library Includes
#include <DirectXCollision.h>

//Local Includes
#include "types.h"
#include "ientity.h"

//Prototypes
class CEntity3D: public IEntity
{
	//Member Functions
public:
	CEntity3D();
	virtual ~CEntity3D();

	virtual void Process(float _fDeltaTick);

	//Position Functions
	virtual void SetPosition(float3 _vec3Position);
	virtual void SetPosition(float _fX, float _fY, float _fZ);
	virtual float3 GetPosition() const;

	//Rotation Functions
	virtual void SetRotation(float3 _vec3PitchYawRoll);
	virtual void SetRotation(float _fPitch, float _fYaw, float _fRoll);
	virtual float3 GetRotation() const;

	//Scale Functions
	virtual void SetScale(float3 _vec3Scale);
	virtual void SetScale(float _fX, float _fY, float _fZ);
	virtual float3 GetScale() const;

	//Bounding Box/Sphere Functions
	DirectX::BoundingOrientedBox GetOBB();
	DirectX::BoundingSphere GetBoundingSphere();

	//Rendering Functions
	void SetRenderOptions(bool _bVisible, bool _bCastShadows, bool _bReceiveShadows);
	bool IsVisible() const;
	bool GetCastShadows() const;
	bool GetReceiveShadows() const;

	//Member Variables
protected:
	float4x4 m_matWorld;
	float3 m_vec3Position;
	float3 m_vec3Rotation;
	float3 m_vec3Scale;
	bool m_bUpdateWorldMatrix; //Pending update for world matrix

	DirectX::BoundingOrientedBox m_tOriginalOBB;
	DirectX::BoundingOrientedBox m_tOBB;
	DirectX::BoundingSphere m_tBoundingSphere;

	bool m_bCastShadow;
	bool m_bReceiveShadow;
	bool m_bVisible;
};

#endif //__ENTITY_3D_H__