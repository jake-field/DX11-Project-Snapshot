#pragma once
#ifndef __FOLLOW_CAMERA_H__
#define __FOLLOW_CAMERA_H__

//Local Includes
#include "camera.h"

//Prototypes
class CEntity3D;
class CFollowCamera: public CCamera
{
	//Member Functions
public:
	CFollowCamera();
	virtual ~CFollowCamera();

	virtual void Process(float _fDeltaTick);

	//Target can be null to revert to a standard camera
	void SetTarget(CEntity3D* _pTargetEntity);
	CEntity3D* GetTarget() const;

	void SetFollowOffset(float3 _vec3Offset, bool _bRelative = true);
	float3 GetFollowOffset() const;

	void SetRotationOffset(float _fYaw, float _fPitch, float _fRoll);
	float3 GetRotationOffset() const;

	bool IsOffsetRelative() const;

	//Member Variables
protected:
	CEntity3D* m_pTarget;

	//Entity Offset Variables
	float3 m_vec3Offset;
	float3 m_vec3RotationOffset;
	bool m_bRelative;

};

#endif //__FOLLOW_CAMERA_H__