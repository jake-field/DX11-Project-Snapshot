#pragma once
#ifndef __CAMERA_H__
#define __CAMERA_H__

//Library Includes
#include <DirectXCollision.h>

//Local Includes
#include "entity3d.h"

//Prototypes
class CRenderer;
class CCamera: public CEntity3D
{
	//Member Functions
public:
	CCamera();
	virtual ~CCamera();

	virtual bool Initialize(CRenderer* _pRenderer);
	virtual void Process(float _fDeltaTick = 1.0f); //TODO: deltatick not used? Temporarily setting this as optional

	//Position, both relative and world
	void ShiftPosition(float _fForward, float _fUp, float _fLeft);
	void LookAt(float3 _vec3Position);

	void SetFOV(float _fFOV, bool _bInDegrees = true);
	float GetFOV(bool _bInDegrees = true) const;

	void SetNearFarPlane(float _fNear, float _fFar);
	float2 GetNearFarPlane() const;

	void SetOrthographicMatrix(float4x4 _matOrtho);
	void SetOrthographicMatrix(float _fWidth, float _fHeight, float _fNear, float _fFar);
	void SetAsOrthogonal(bool _bUseOrthogonal);
	bool IsOrthogonal() const;

	void SetAsActiveCamera();
	static void SetActiveCamera(CCamera* _pCamera);
	static CCamera* GetActiveCamera();

	//If _bScaleInPercent is true, then viewport width/height will be treated as a % of window width/height (0.0f through 1.0f)
	void SetViewport(const D3D11_VIEWPORT& _rtViewport, bool _bScaleInPercent);
	D3D11_VIEWPORT GetViewport() const;

	//Get functions for look/eye
	float3 GetLook() const;
	float3 GetRight() const;
	float3 GetUp() const;
	float3 GetEyePos() const; //Duplicate of GetPosition(), alternative name

	//Retrieves the matrices created by the camera
	float4x4 GetViewMatrix() const;
	float4x4 GetViewProjMatrix() const;
	float4x4 GetPerspectiveMatrix() const;
	float4x4 GetOrthogonalMatrix() const;
	float4x4 GetProjectionMatrix() const;

	//Bounding view frustum
	const DirectX::BoundingFrustum& GetBoundingFrustum() const;

protected:
	void BuildViewMatrix();

private:
	//TODO: Debug frustrum drawing etc. Maybe camera mesh in editors? Again consider ECS system
	//Hide non-relevant functions
	virtual void SetScale(float3 _vec3Scale) {};
	virtual void SetScale(float _fX, float _fY, float _fZ) {};
	virtual float3 GetScale() const { return(m_vec3Scale); };
	virtual void Draw() {}; //Not a drawable entity

	//Member Variables
protected:
	//Camera registering
	static CCamera* sm_pActiveCamera;

	//External class pointers
	CRenderer* m_pRenderer;

	//Camera Matrices
	float4x4 m_matViewProjection;
	float4x4 m_matPerspective;
	float4x4 m_matOrthogonal;
	float4x4 m_matView;

	//Viewport
	D3D11_VIEWPORT m_tViewport;
	float2 m_vec2ViewportScale;

	//Near Far values for projection matrices
	float2 m_vec2NearFar;

	//Camera Look Variables
	float3 m_vec3Up;
	float3 m_vec3Look;
	float3 m_vec3Right;

	//Misc Variables
	float m_fFOV;
	bool m_bIsOrthogonal;
	bool m_bViewportAsPercent;
	bool m_bUpdateProjMatrix;
	bool m_bUpdateViewProjMatrix;
	bool m_bUpdateViewport;
	DirectX::BoundingFrustum m_tViewFrustum;

};

#endif //__CAMERA_H__