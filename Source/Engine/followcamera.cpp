//Local Includes
#include "entity3d.h"

//This Include
#include "followcamera.h"

//Implementation
CFollowCamera::CFollowCamera()
	: m_pTarget(nullptr)
	, m_vec3Offset(0.0f, 0.0f, 0.0f)
	, m_vec3RotationOffset(0.0f, 0.0f, 0.0f)
	, m_bRelative(true)
{
	//Constructor
}

CFollowCamera::~CFollowCamera()
{
	//Destructor
}

void
CFollowCamera::Process(float _fDeltaTick)
{
	//Bound to entity?
	if(m_pTarget != nullptr)
	{
		//Relative to entity?
		if(m_bRelative)
		{
			//Offset is relative to entity rotation
			m_vec3Rotation = m_pTarget->GetRotation() + m_vec3RotationOffset;
		}
		else
		{
			//Using camera's world YawPitchRoll
		}

		//Set to entity position and apply offset
		m_vec3Position = m_pTarget->GetPosition();
		ShiftPosition(m_vec3Offset.x, m_vec3Offset.y, m_vec3Offset.z);
		m_bUpdateWorldMatrix = true; //Even though ShiftPosition() sets this, sanity check.
	}

	//Call up to CCamera
	__super::Process(_fDeltaTick);
}

void
CFollowCamera::SetTarget(CEntity3D* _pTargetEntity)
{
	//Avoid binding to self as it makes zero sense (and will cause issues)
	if(_pTargetEntity != this) m_pTarget = _pTargetEntity;
}

CEntity3D*
CFollowCamera::GetTarget() const
{
	return(m_pTarget);
}

void
CFollowCamera::SetFollowOffset(float3 _vec3Offset, bool _bRelative)
{
	//Is the offset Relative to the object's rotation, or is it just YawPitchRoll based (World).
	m_bRelative = _bRelative;
	m_vec3Offset = _vec3Offset;
	m_bUpdateWorldMatrix = true;
}

float3
CFollowCamera::GetFollowOffset() const
{
	return(m_vec3Offset);
}

void
CFollowCamera::SetRotationOffset(float _fPitch, float _fYaw, float _fRoll)
{
	m_vec3RotationOffset.x = _fPitch;
	m_vec3RotationOffset.y = _fYaw;
	m_vec3RotationOffset.z = _fRoll;
	m_bUpdateWorldMatrix = true;
}

float3
CFollowCamera::GetRotationOffset() const
{
	return(m_vec3RotationOffset);
}

bool
CFollowCamera::IsOffsetRelative() const
{
	return(m_bRelative);
}
