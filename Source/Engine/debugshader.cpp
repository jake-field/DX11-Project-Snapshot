//Local Includes
#include "renderer.h"

//This Include
#include "debugshader.h"

//Implementation
CDebugShader::CDebugShader()
	: m_pCBuffers(nullptr)
	, m_iCBufferCount(0)
{
	//Constructor
}

CDebugShader::~CDebugShader()
{
	//Destructor

	//Release the constant buffers
	if(m_pCBuffers)
	{
		for(int i = 0; i < m_iCBufferCount; ++i) ReleaseCOM(m_pCBuffers[i]);

		//Delete the storage array
		SafeDelete(m_pCBuffers);
	}
}

bool
CDebugShader::Initialize(CRenderer* _pRenderer)
{
	m_pRenderer = _pRenderer;

	TVertexLayoutSemantic tLayout[] =
	{
		{"POSITION", DXGI_FORMAT_R32G32B32_FLOAT}, //TODO: Add COLOR
	};

	//TODO: load uncompiled if cso no available and compile to CSO
	//Load Shader
	TShaderFileDesc tDebugVS("Resources\\Shaders\\debug_vs.cso");
	TShaderFileDesc tDebugPS("Resources\\Shaders\\debug_ps.cso");
	LoadFromFile(EShaderType::VERTEX, 0, tDebugVS, tLayout, SizeofArray(tLayout));
	LoadFromFile(EShaderType::PIXEL, 0, tDebugPS);

	//Empty CBuffer fills
	TCBufferDebugPerObject tCBPerObject;
	ZeroMemory(&tCBPerObject, sizeof(TCBufferDebugPerObject));

	//Create constant buffers
	m_iCBufferCount = 1;
	m_pCBuffers = new ID3D11Buffer * [m_iCBufferCount];
	m_pCBuffers[0] = m_pRenderer->CreateBuffer(D3D11_BIND_CONSTANT_BUFFER, &tCBPerObject, sizeof(TCBufferDebugPerObject), D3D11_USAGE_DYNAMIC);

	//Generate Cube
	float3 pCubeVerts[] = {	float3(0.5f,  0.5f, -0.5f),
							float3(0.5f, -0.5f, -0.5f),
							float3(-0.5f, -0.5f, -0.5f),
							float3(-0.5f,  0.5f, -0.5f),
							float3(0.5f,  0.5f,  0.5f),
							float3(0.5f, -0.5f,  0.5f),
							float3(-0.5f, -0.5f,  0.5f),
							float3(-0.5f,  0.5f,  0.5f)};
	DWORD pCubeFaces[] = {	0,1,1,2,2,3,3,0, //Top
							4,5,5,6,6,7,7,4, //Bottom
							0,4,1,5,2,6,3,7};//Sides

	//TODO: Generate Sphere
	int iColumns = 4;
	int iRows = 1;
	int iSubdivisions = 5;
	float fRadius = 0.5f;

	//Create debug meshes
	TMeshData<float3> tCubeData(pCubeVerts, 8, pCubeFaces, 24);
	//TMeshData<float3> tSphereData(nullptr, 0);
	m_meshCube.Initialize(m_pRenderer, tCubeData);
	m_meshCube.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//m_meshSphere.Initialize(m_pRenderer, tSphereData);


	//TODO: fix this
	return false;
}

bool
CDebugShader::ApplyShader()
{
	//CDX11Shader::ApplyShader();
	//Returns true if we were the last shader active
	bool bAlreadyActive = __super::ApplyShader(); //Pass doesnt matter to DX11Shader

	//Required to update bound shaders and pass settings
	__super::SetPass(0);

	return(false);
}

void
CDebugShader::FinishShader()
{
	//CDX11Shader::FinishShader()
	__super::FinishShader();
}

void
CDebugShader::DrawCube(float3 _vec3pos, float3 _vec3rot, float3 _vec3scale)
{
	float4x4 matWorld;

	float3 vec3RadRot = float3(XMConvertToRadians(_vec3rot.x), XMConvertToRadians(_vec3rot.y), XMConvertToRadians(_vec3rot.z));
	XMMATRIX matTemp = XMMatrixIdentity();
	XMVECTOR xvecPos = XMLoadFloat3(&_vec3pos);
	XMVECTOR xvecScale = XMLoadFloat3(&_vec3scale);
	XMVECTOR xvecRot = XMLoadFloat3(&vec3RadRot);

	//Scale, Rotate, Move
	matTemp = XMMatrixScalingFromVector(xvecScale);
	matTemp = XMMatrixMultiply(matTemp, XMMatrixRotationRollPitchYawFromVector(xvecRot));
	matTemp = XMMatrixMultiply(matTemp, XMMatrixTranslationFromVector(xvecPos));
	XMStoreFloat4x4(&matWorld, matTemp);

	//Call for CMesh as there are some extra steps
	m_meshCube.Draw(&matWorld);
}

void
CDebugShader::DrawCube(float3 _vec3pos, float4 _quatRot, float3 _vec3scale)
{
	float4x4 matWorld;

	XMMATRIX matTemp = XMMatrixIdentity();
	XMVECTOR xvecPos = XMLoadFloat3(&_vec3pos);
	XMVECTOR xvecScale = XMLoadFloat3(&_vec3scale);
	XMVECTOR xvecRot = XMLoadFloat4(&_quatRot);

	//Scale, Rotate, Move
	matTemp = XMMatrixScalingFromVector(xvecScale);
	matTemp = XMMatrixMultiply(matTemp, XMMatrixRotationQuaternion(xvecRot));
	matTemp = XMMatrixMultiply(matTemp, XMMatrixTranslationFromVector(xvecPos));
	XMStoreFloat4x4(&matWorld, matTemp);

	//Call for CMesh as there are some extra steps
	m_meshCube.Draw(&matWorld);
}

void
CDebugShader::DrawSphere(float3 _vec3pos, float3 _vec3rot, float3 _vec3scale)
{
	float4x4 matWorld;

	float3 vec3RadRot = float3(XMConvertToRadians(_vec3rot.x), XMConvertToRadians(_vec3rot.y), XMConvertToRadians(_vec3rot.z));
	XMMATRIX matTemp = XMMatrixIdentity();
	XMVECTOR xvecPos = XMLoadFloat3(&_vec3pos);
	XMVECTOR xvecScale = XMLoadFloat3(&_vec3scale);
	XMVECTOR xvecRot = XMLoadFloat3(&vec3RadRot);

	//Scale, Rotate, Move
	matTemp = XMMatrixScalingFromVector(xvecScale);
	matTemp = XMMatrixMultiply(matTemp, XMMatrixRotationRollPitchYawFromVector(xvecRot));
	matTemp = XMMatrixMultiply(matTemp, XMMatrixTranslationFromVector(xvecPos));
	XMStoreFloat4x4(&matWorld, matTemp);

	//Call for CMesh as there are some extra steps
	m_meshSphere.Draw(&matWorld);
}

void
CDebugShader::DrawSphere(float3 _vec3pos, float3 _vec3rot, float _fRadius)
{
	DrawSphere(_vec3pos, _vec3rot, float3(_fRadius, _fRadius, _fRadius));
}

bool
CDebugShader::Predraw(const IMesh* _pMesh, const float4x4* _ptWorldMatrix, bool _bInstanced)
{
	bool bSuccessful = false;

	bool bIsActiveShader = (sm_pActiveShader == this);
	bool bRendererReady = m_pRenderer && m_pRenderer->GetDeviceContext() && m_pRenderer->IsSceneActive();
	bool bValidParams = _pMesh && _ptWorldMatrix; //nullptr check

	if(bIsActiveShader && bRendererReady && bValidParams)
	{
		//Get Pass Info
		TShaderPass tPass = m_vecPasses[0];

		//Bind vertex layout
		m_pRenderer->GetDeviceContext()->IASetInputLayout(tPass.pVertexLayout);

		//Build the cbuffer
		TCBufferDebugPerObject tCBPerObject;
		tCBPerObject.matWorld = _ptWorldMatrix->Transpose();

		//Fill the per-object cbuffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		HRESULT hrMapped = m_pRenderer->GetDeviceContext()->Map(m_pCBuffers[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		if(SUCCEEDED(hrMapped))
		{
			memcpy_s(MappedResource.pData, sizeof(TCBufferDebugPerObject), &tCBPerObject, sizeof(TCBufferDebugPerObject));
			m_pRenderer->GetDeviceContext()->Unmap(m_pCBuffers[0], 0);
		}

		//Apply the per-object cbuffer data to register(b2)
		int iCbSlot = (int)EDebugShaderBindings::CB_PEROBJECT;
		if(tPass.pVertexShader) m_pRenderer->GetDeviceContext()->VSSetConstantBuffers(iCbSlot, 1, &m_pCBuffers[0]);
		if(tPass.pPixelShader) m_pRenderer->GetDeviceContext()->PSSetConstantBuffers(iCbSlot, 1, &m_pCBuffers[0]);

		bSuccessful = true; //TODO: Add checks, but only the Map/Unmap returns a state
	}
	else if(!bRendererReady)
	{
		//Renderer not ready error
	}
	else if(bIsActiveShader)
	{
		//Unable to draw due to shader not ready error
	}
	else //!bCanDraw
	{
		//Primitive not drawn
	}

	return(bSuccessful);
}
