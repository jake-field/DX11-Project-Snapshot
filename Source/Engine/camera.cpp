//Local Includes
#include "renderer.h"

//This Include
#include "camera.h"

//Static Declarations
CCamera* CCamera::sm_pActiveCamera = nullptr;

//Implementation
CCamera::CCamera()
	: m_pRenderer(nullptr)
	, m_tViewport(CD3D11_VIEWPORT(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f)) //Make camera use 100% of screen size w/ m_bViewportAsPercent(true)
	, m_vec2ViewportScale(1.0f, 1.0f) //100% window scale
	, m_vec2NearFar(1.0f, 1000.0f)
	, m_vec3Up(0.0f, 1.0f, 0.0f)
	, m_vec3Look(0.0f, 0.0f, 1.0f)
	, m_vec3Right(1.0f, 0.0f, 0.0f)
	, m_fFOV(0.25f * XM_PI)
	, m_bIsOrthogonal(false)
	, m_bViewportAsPercent(true) //Ensure viewport is calculated using m_vec2ViewportScale
	, m_bUpdateProjMatrix(true)
	, m_bUpdateViewProjMatrix(true)
	, m_bUpdateViewport(true) //Force viewport update
{
	//Constructor
	XMStoreFloat4x4(&m_matViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_matPerspective, XMMatrixIdentity());
	XMStoreFloat4x4(&m_matOrthogonal, XMMatrixIdentity());
	XMStoreFloat4x4(&m_matView, XMMatrixIdentity());
}

CCamera::~CCamera()
{
	//Destructor
	//If we are the active camera, make sure to unset the active camera...
	if(sm_pActiveCamera == this) sm_pActiveCamera = nullptr;
}

bool
CCamera::Initialize(CRenderer* _pRenderer)
{
	//TODO: consider this function
	//Could do active renderer? IRenderer::GetActiveRenderer() etc.
	//Or just leave it up to other classes to pull camera info and insert into renderer.
	m_pRenderer = _pRenderer;
	return(m_pRenderer != nullptr);
}

void
CCamera::Process(float _fDeltaTick)
{
	bool bActiveCamera = (sm_pActiveCamera == this);
	if(!bActiveCamera) m_bUpdateViewport = true; //When we gain control, make sure the renderer viewport is updated

	//Check if viewport is incorrect (only works if done as a percentage, otherwise rely on external updates to viewport width/height
	//This will fire when renderer/window resolution changes, so it needs to remain independent
	if(m_bViewportAsPercent)
	{
		//Check if window size differs to what has been set
		float2 vec2RendererSize = m_pRenderer->GetSize();
		float2 vec2DesiredViewport = float2(vec2RendererSize.x * m_vec2ViewportScale.x, vec2RendererSize.y * m_vec2ViewportScale.y);

		//If an update is pending (sanity) or viewport width/height varies at all, update the width/height of the viewport
		if(m_bUpdateViewport || vec2DesiredViewport.x != m_tViewport.Width || vec2DesiredViewport.y != m_tViewport.Height)
		{
			m_tViewport.Width = vec2DesiredViewport.x; //Set according to window width
			m_tViewport.Height = vec2DesiredViewport.y; //Set according to window height
			m_bUpdateProjMatrix = true;
			m_bUpdateViewport = true;
		}
	}

	//Viewport update
	if(m_bUpdateViewport && bActiveCamera) //Only update when we are active as it changes the viewport on the renderer
	{
		//Set the viewport on the renderer
		m_pRenderer->GetDeviceContext()->RSSetViewports(1, &m_tViewport);
		m_bUpdateViewport = false; //Nothing else required
	}

	//Projection matrix update
	if(m_bUpdateProjMatrix)
	{
		XMStoreFloat4x4(&m_matPerspective, XMMatrixPerspectiveFovLH(m_fFOV, (m_tViewport.Width / m_tViewport.Height), m_vec2NearFar.x, m_vec2NearFar.y));
	}

	//View matrix update
	if(m_bUpdateWorldMatrix) //If the entity or camera is updated, rebuild the view matrix as we would a world matrix
	{
		BuildViewMatrix();
	}

	//If view or projection matrix were updated
	if(m_bUpdateProjMatrix || m_bUpdateViewProjMatrix || m_bUpdateWorldMatrix)
	{
		//Update View*Projection mat
		XMMATRIX matProj = XMLoadFloat4x4(m_bIsOrthogonal ? &m_matOrthogonal : &m_matPerspective);
		
		XMStoreFloat4x4(&m_matViewProjection, XMMatrixMultiply(XMLoadFloat4x4(&m_matView), matProj));

		//Recreate bounding frustum when project matrix changes
		if(m_bUpdateProjMatrix)
		{
			//Only call this if we absolutely have to
			DirectX::BoundingFrustum::CreateFromMatrix(m_tViewFrustum, matProj);
		}

		//Update bounding frustum position and orientation (cleared by CreateFromMatrix)
		m_tViewFrustum.Origin = m_vec3Position;
		XMStoreFloat4(&m_tViewFrustum.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_vec3Rotation.x), XMConvertToRadians(m_vec3Rotation.y), XMConvertToRadians(m_vec3Rotation.z)));

		//States to false
		m_bUpdateProjMatrix = false;
		m_bUpdateViewProjMatrix = false;

		//Do not set this. It is set during CEntity3D::Process once matworld is updated.
		//m_bUpdateWorldMatrix = false;
	}

	//Call CEntity3D::Process() just to prevent future issues where m_matworld is used (maybe for debug camera meshes in editors etc.)
	__super::Process(_fDeltaTick);
}

void
CCamera::ShiftPosition(float _fForward, float _fUp, float _fLeft)
{
	//Move camera postion by X, Y, Z rather than outright setting position
	m_vec3Position += m_vec3Look * _fForward;
	m_vec3Position += m_vec3Up * _fUp;
	m_vec3Position += m_vec3Right * _fLeft;
	m_bUpdateWorldMatrix = true; //Forces view matrix update as well
}

void
CCamera::LookAt(float3 _vec3Position)
{
	//Reset camera to prevent issues
	XMStoreFloat4x4(&m_matView, XMMatrixIdentity());
	m_vec3Up = float3(0.0f, 1.0f, 0.0f);
	m_vec3Look = float3(0.0f, 0.0f, 1.0f);
	m_vec3Right = float3(1.0f, 0.0f, 0.0f);

	//Store view matrix
	XMStoreFloat4x4(&m_matView, XMMatrixLookAtLH(XMLoadFloat3(&m_vec3Position), XMLoadFloat3(&_vec3Position), XMLoadFloat3(&m_vec3Up)));

	//Store rotation
	m_vec3Rotation = float3(XMConvertToDegrees(asinf(-m_matView._23)), XMConvertToDegrees(atan2f(m_matView._13, m_matView._33)), XMConvertToDegrees(atan2f(m_matView._21, m_matView._22)));

	//Copy the new up/look vectors back into float3
	m_vec3Right = float3(m_matView._11, m_matView._21, m_matView._31);
	m_vec3Up = float3(m_matView._12, m_matView._22, m_matView._32);
	m_vec3Look = float3(m_matView._13, m_matView._23, m_matView._33);

	//Force matVP update, skip matView update as we already did that
	m_bUpdateViewProjMatrix = true;
}

void
CCamera::SetFOV(float _fFOV, bool _bInDegrees)
{
	m_fFOV = _bInDegrees ? XMConvertToRadians(_fFOV) : _fFOV;
	m_bUpdateProjMatrix = true; //FOV affects perspective matrix, requires update
}

float
CCamera::GetFOV(bool _bInDegrees) const
{
	return(_bInDegrees ? XMConvertToDegrees(m_fFOV) : m_fFOV);
}

void
CCamera::SetNearFarPlane(float _fNear, float _fFar)
{
	m_vec2NearFar = float2(_fNear, _fFar);
	m_bUpdateProjMatrix = true;
}

float2
CCamera::GetNearFarPlane() const
{
	return(m_vec2NearFar);
}

void
CCamera::SetOrthographicMatrix(float4x4 _matOrtho)
{
	m_matOrthogonal = _matOrtho;
}

void
CCamera::SetOrthographicMatrix(float _fWidth, float _fHeight, float _fNear, float _fFar)
{
	XMStoreFloat4x4(&m_matOrthogonal, XMMatrixOrthographicLH(_fWidth, _fHeight, _fNear, _fFar));
}

void
CCamera::SetAsOrthogonal(bool _bUseOrthogonal)
{
	m_bIsOrthogonal = _bUseOrthogonal;
	m_bUpdateViewProjMatrix = true; //ViewProj needs updating
}

bool
CCamera::IsOrthogonal() const
{
	return(m_bIsOrthogonal);
}

void
CCamera::SetAsActiveCamera()
{
	//Non-static function
	sm_pActiveCamera = this;
	m_bUpdateViewport = true; //Force set active viewport

	//Force an update on camera change
	Process();
}

void
CCamera::SetActiveCamera(CCamera* _pCamera)
{
	//Static function
	sm_pActiveCamera = _pCamera;
	if(sm_pActiveCamera != nullptr) sm_pActiveCamera->m_bUpdateViewport = true; //Force set active viewport on the camera if there is one
}

CCamera*
CCamera::GetActiveCamera()
{
	//Static function
	return(sm_pActiveCamera);
}

void
CCamera::SetViewport(const D3D11_VIEWPORT& _rtViewport, bool _bScaleInPercent)
{
	m_bViewportAsPercent = _bScaleInPercent; //m_tViewport width/height is percentage of window size, not pixels
	m_tViewport = _rtViewport; //Copy

	//If treated as a percentage
	if(m_bViewportAsPercent)
	{
		m_vec2ViewportScale = float2(_rtViewport.Width, _rtViewport.Height); //Store % requested

		//Debug check for "percentage" issues and my sanity
		//TODO: remove this code if not needed
		#ifdef _DEBUG
		if(m_vec2ViewportScale.x > 1.0f || m_vec2ViewportScale.y > 1.0f)
		{
			OutputDebugString(L"Camera viewport width/height parsed as scale, but scale exceeds range of (0.0f, 1.0f). Is this intentional?\n");
		}
		#endif //_DEBUG
	}
	else
	{
		//Assume viewport requested is in pixels and correct
	}

	//TODO: viewport sanity check, no negative numbers, within window resolution?

	//Projection matrices will need updating as well as Rasterstate
	m_bUpdateProjMatrix = true;
	m_bUpdateViewport = true;
}

D3D11_VIEWPORT
CCamera::GetViewport() const
{
	return(m_tViewport);
}

float3
CCamera::GetLook() const
{
	return(m_vec3Look);
}

float3
CCamera::GetRight() const
{
	return(m_vec3Right);
}

float3
CCamera::GetUp() const
{
	return(m_vec3Up);
}

float3
CCamera::GetEyePos() const
{
	return(m_vec3Position);
}

float4x4
CCamera::GetViewMatrix() const
{
	return(m_matView);
}

float4x4
CCamera::GetViewProjMatrix() const
{
	return(m_matViewProjection);
}

float4x4
CCamera::GetPerspectiveMatrix() const
{
	return(m_matPerspective);
}

float4x4
CCamera::GetOrthogonalMatrix() const
{
	return(m_matOrthogonal);
}

float4x4
CCamera::GetProjectionMatrix() const
{
	return(m_bIsOrthogonal ? m_matOrthogonal : m_matPerspective);
}

const DirectX::BoundingFrustum&
CCamera::GetBoundingFrustum() const
{
	return(m_tViewFrustum);
}

void
CCamera::BuildViewMatrix()
{
	//Reset camera to prevent issues
	XMStoreFloat4x4(&m_matView, XMMatrixIdentity());
	m_vec3Up = float3(0.0f, 1.0f, 0.0f);
	m_vec3Look = float3(0.0f, 0.0f, 1.0f);
	m_vec3Right = float3(1.0f, 0.0f, 0.0f);

	//Load float vectors into XMVECTOR
	XMVECTOR xvec3Up = XMLoadFloat3(&m_vec3Up);
	XMVECTOR xvec3Position = XMLoadFloat3(&m_vec3Position);
	XMVECTOR xvec3Look = XMLoadFloat3(&m_vec3Look);

	//Create rotation matrix
	XMMATRIX matRotation = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_vec3Rotation.x), XMConvertToRadians(m_vec3Rotation.y), XMConvertToRadians(m_vec3Rotation.z));

	//Transfoorm Look and Up vectors to align with camera rotation
	xvec3Look = XMVector3TransformCoord(xvec3Look, matRotation);
	xvec3Up = XMVector3TransformCoord(xvec3Up, matRotation);

	//Copy the new up/look vectors back into float3
	XMStoreFloat3(&m_vec3Up, xvec3Up);
	XMStoreFloat3(&m_vec3Look, xvec3Look);
	XMStoreFloat3(&m_vec3Right, XMVector3Cross(xvec3Up, xvec3Look));

	//Store new View Matrix
	XMStoreFloat4x4(&m_matView, XMMatrixLookToLH(xvec3Position, xvec3Look, xvec3Up));
}
