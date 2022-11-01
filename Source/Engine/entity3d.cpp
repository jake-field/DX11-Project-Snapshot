//This Include
#include "entity3d.h"

//Implementation
CEntity3D::CEntity3D()
	: m_vec3Position(0.0f, 0.0f, 0.0f)
	, m_vec3Rotation(0.0f, 0.0f, 0.0f)
	, m_vec3Scale(1.0f, 1.0f, 1.0f)
	, m_bUpdateWorldMatrix(true)
	, m_bReceiveShadow(true)
	, m_bCastShadow(true)
	, m_bVisible(true)
{
	//Constructor
	XMStoreFloat4x4(&m_matWorld, XMMatrixIdentity());
}

CEntity3D::~CEntity3D()
{
	//Destructor
}

void
CEntity3D::Process(float _fDeltaTick)
{
	//Only update the world matrix if there has been a change
	if(m_bUpdateWorldMatrix)
	{
		float3 vec3RadRot = float3(XMConvertToRadians(m_vec3Rotation.x), XMConvertToRadians(m_vec3Rotation.y), XMConvertToRadians(m_vec3Rotation.z));
		XMMATRIX matWorld = XMMatrixIdentity();
		XMVECTOR xvecPos = XMLoadFloat3(&m_vec3Position);
		XMVECTOR xvecScale = XMLoadFloat3(&m_vec3Scale);
		XMVECTOR xvecRot = XMLoadFloat3(&vec3RadRot);

		//Scale, Rotate, Move
		matWorld = XMMatrixScalingFromVector(xvecScale);
		matWorld = XMMatrixMultiply(matWorld, XMMatrixRotationRollPitchYawFromVector(xvecRot));
		matWorld = XMMatrixMultiply(matWorld, XMMatrixTranslationFromVector(xvecPos));
		XMStoreFloat4x4(&m_matWorld, matWorld);

		//Update OBB 
		m_tOriginalOBB.Transform(m_tOBB, matWorld);
		m_tBoundingSphere.CreateFromBoundingBox(m_tBoundingSphere, m_tOBB);

		m_bUpdateWorldMatrix = false; //Revert
	}
}

void
CEntity3D::SetPosition(float3 _vec3Position)
{
	m_vec3Position = _vec3Position;
	m_bUpdateWorldMatrix = true;
}

void
CEntity3D::SetPosition(float _fX, float _fY, float _fZ)
{
	m_vec3Position.x = _fX;
	m_vec3Position.y = _fY;
	m_vec3Position.z = _fZ;
	m_bUpdateWorldMatrix = true;
}

void
CEntity3D::SetRotation(float3 _vec3PitchYawRoll)
{
	m_vec3Rotation = _vec3PitchYawRoll;
	m_bUpdateWorldMatrix = true;
}

void
CEntity3D::SetRotation(float _fPitch, float _fYaw, float _fRoll)
{
	m_vec3Rotation.x = _fPitch;
	m_vec3Rotation.y = _fYaw;
	m_vec3Rotation.z = _fRoll;
	m_bUpdateWorldMatrix = true;
}

void
CEntity3D::SetScale(float3 _vec3Scale)
{
	m_vec3Scale = _vec3Scale;
	m_bUpdateWorldMatrix = true;
}

void
CEntity3D::SetScale(float _fX, float _fY, float _fZ)
{
	m_vec3Scale.x = _fX;
	m_vec3Scale.y = _fY;
	m_vec3Scale.z = _fZ;
	m_bUpdateWorldMatrix = true;
}

float3
CEntity3D::GetPosition() const
{
	return(m_vec3Position);
}

float3
CEntity3D::GetRotation() const
{
	return(m_vec3Rotation);
}

float3
CEntity3D::GetScale() const
{
	return(m_vec3Scale);
}

DirectX::BoundingOrientedBox
CEntity3D::GetOBB()
{
	return(m_tOBB);
}

DirectX::BoundingSphere
CEntity3D::GetBoundingSphere()
{
	return(m_tBoundingSphere);
}

void
CEntity3D::SetRenderOptions(bool _bVisible, bool _bCastShadows, bool _bReceiveShadows)
{
	m_bVisible = _bVisible;
	m_bCastShadow = _bCastShadows;
	m_bReceiveShadow = _bReceiveShadows;
}

bool
CEntity3D::IsVisible() const
{
	return(m_bVisible);
}

bool
CEntity3D::GetCastShadows() const
{
	return(m_bCastShadow);
}

bool
CEntity3D::GetReceiveShadows() const
{
	return(m_bReceiveShadow);
}
