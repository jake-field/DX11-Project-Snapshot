//Local Includes
#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "imesh.h"
#include "material.h"
#include "texture.h"
#include "shaderglobals.h"

//This Include
#include "defaultshader.h"

//Implementation
CDefaultShader::CDefaultShader()
	: m_pSceneCamera(nullptr)
	, m_pSunLight(nullptr)
	, m_pCBuffers(nullptr)
	, m_pShadowMapTexture(nullptr)
	, m_pShadowMapDSV(nullptr)
	, m_pShadowMapSRV(nullptr)
	, m_prsShadow(nullptr)
	, m_iActivePass(-1)
	, m_iCBufferCount(0)
	, m_pErrorTex(nullptr)
	, m_pBlackTex(nullptr)
	, m_pWhiteTex(nullptr)
{
	//Constructor
}

CDefaultShader::~CDefaultShader()
{
	//Destructor
	m_pSceneCamera = nullptr;
	SafeDelete(m_pSunLight);
	m_iActivePass = -1;

	m_pBlackTex = nullptr;
	m_pErrorTex = nullptr;
	m_pWhiteTex = nullptr;

	//Release the constant buffers
	if(m_pCBuffers)
	{
		for(int i = 0; i < m_iCBufferCount; ++i) ReleaseCOM(m_pCBuffers[i]);

		//Delete the storage array
		SafeDelete(m_pCBuffers);
	}

	//Shader Resources
	ReleaseCOM(m_pShadowMapTexture);
	ReleaseCOM(m_pShadowMapDSV);
	ReleaseCOM(m_pShadowMapSRV);
	ReleaseCOM(m_prsShadow);
}

bool
CDefaultShader::Initialize(CRenderer* _pRenderer, CCamera* _pSceneCamera)
{
	const int kiShadowmapResolution = 4096; //TODO: Make custom?
	//const int kiShadowmapResolution = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION; //TODO: Make custom?

	m_pRenderer = _pRenderer;
	
	//TODO: load uncompiled if cso no available and compile to CSO
	//Load Shaders
	TVertexLayoutSemantic tLayout[] =
	{
		{"POSITION", DXGI_FORMAT_R32G32B32_FLOAT},
		{"NORMAL", DXGI_FORMAT_R32G32B32_FLOAT},
		{"TANGENT", DXGI_FORMAT_R32G32B32_FLOAT},
		{"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT},
	};

	TShaderFileDesc tPass0vs("Resources\\Shaders\\default_p0_vs.cso"); //shadowmap
	TShaderFileDesc tPass1vs("Resources\\Shaders\\default_p1_vs.cso"); //tex
	TShaderFileDesc tPass1ps("Resources\\Shaders\\default_p1_ps.cso"); //tex
	LoadFromFile(EShaderType::VERTEX, 0, tPass0vs, tLayout, SizeofArray(tLayout));
	LoadFromFile(EShaderType::VERTEX, 1, tPass1vs, tLayout, SizeofArray(tLayout));
	LoadFromFile(EShaderType::PIXEL, 1, tPass1ps);

	//Using third and fourth pass for the instance shaders so they don't accidentally get set
	//TODO: Consider alternatives to this pass system, we should allow pass stacking (multiple VS/PS per pass)
	TVertexLayoutSemantic tInstanceLayout[] =
	{
		tLayout[0], tLayout[1], tLayout[2], tLayout[3], //Pre-fil with the non-instanced semantic to save mismatches
		{"I_POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0, 1, 1},
		{"I_SCALE", DXGI_FORMAT_R32G32B32_FLOAT, 0, 1, 1},
		{"I_ROTATION", DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 1, 1},
	};

	TShaderFileDesc tPass0vsinstance("Resources\\Shaders\\default_p0_vsinstance.cso");
	TShaderFileDesc tPass1vsinstance("Resources\\Shaders\\default_p1_vsinstance.cso");
	LoadFromFile(EShaderType::VERTEX, 2, tPass0vsinstance, tInstanceLayout, SizeofArray(tInstanceLayout));
	LoadFromFile(EShaderType::VERTEX, 3, tPass1vsinstance, tInstanceLayout, SizeofArray(tInstanceLayout));

	//Empty CBuffer fills
	TCBufferScenePerFrame tCBPerFrame;
	TCBufferScenePerObject tCBPerObject;
	ZeroMemory(&tCBPerFrame, sizeof(TCBufferScenePerFrame));
	ZeroMemory(&tCBPerObject, sizeof(TCBufferScenePerObject));

	//Create constant buffers
	m_iCBufferCount = 2;
	m_pCBuffers = new ID3D11Buffer*[m_iCBufferCount];
	m_pCBuffers[0] = m_pRenderer->CreateBuffer(D3D11_BIND_CONSTANT_BUFFER, &tCBPerFrame, sizeof(TCBufferScenePerFrame), D3D11_USAGE_DYNAMIC);
	m_pCBuffers[1] = m_pRenderer->CreateBuffer(D3D11_BIND_CONSTANT_BUFFER, &tCBPerObject, sizeof(TCBufferScenePerObject), D3D11_USAGE_DYNAMIC);

	//Shadowmap specific
	//Set up viewport values for the shadowmap texture output
	D3D11_VIEWPORT tViewport;
	ZeroMemory(&tViewport, sizeof(D3D11_VIEWPORT));
	tViewport.Width = kiShadowmapResolution;
	tViewport.Height = kiShadowmapResolution;
	tViewport.MinDepth = 0.0f;
	tViewport.MaxDepth = 1.0f;

	//Temp Ortho Matrix
	float4x4 matTempOrtho;
	XMStoreFloat4x4(&matTempOrtho, XMMatrixOrthographicLH(50.0f, 50.0f, 1.0f, 30.0f));

	//Create the light used for shadowmapping
	m_pSunLight = new CLight;
	m_pSunLight->Initialize(_pRenderer);
	m_pSunLight->SetAsOrthogonal(true);
	m_pSunLight->SetOrthographicMatrix(matTempOrtho);
	m_pSunLight->SetViewport(tViewport, false);
	m_pSunLight->SetAsActiveCamera();
	m_pSunLight->Process();

	//Store and set scene camera
	m_pSceneCamera = _pSceneCamera;
	m_pSceneCamera->SetAsActiveCamera();

	//Create the shadowmap texture, stencil view and shader resource
	CD3D11_SHADER_RESOURCE_VIEW_DESC dsrvd(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT, 0, 1);
	CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT);
	CD3D11_TEXTURE2D_DESC dtd(DXGI_FORMAT_R32_TYPELESS, //shadowmap format (32bit)
		kiShadowmapResolution, //shadowmap width
		kiShadowmapResolution, //shadowmap height
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

	m_pRenderer->GetDevice()->CreateTexture2D(&dtd, nullptr, &m_pShadowMapTexture);
	m_pRenderer->GetDevice()->CreateDepthStencilView(m_pShadowMapTexture, &dsvd, &m_pShadowMapDSV);
	m_pRenderer->GetDevice()->CreateShaderResourceView(m_pShadowMapTexture, &dsrvd, &m_pShadowMapSRV);

	//Create the raster state
	CD3D11_RASTERIZER_DESC drd(D3D11_FILL_SOLID,
		D3D11_CULL_NONE,
		FALSE,
		0,
		0.f,
		0.f,
		TRUE,
		FALSE,
		TRUE,
		FALSE);

	//Setting the slope scale depth biase greatly decreases surface acne and incorrect self shadowing.
	drd.SlopeScaledDepthBias = 1.0;
	drd.CullMode = D3D11_CULL_NONE;
	drd.DepthClipEnable = false; //For pancaking
	m_pRenderer->GetDevice()->CreateRasterizerState(&drd, &m_prsShadow);

	//TODO: fix this
	return false;
}

bool
CDefaultShader::ApplyShader(int _iPass)
{
	//CDX11Shader::ApplyShader();
	//Returns true if we were the last shader active
	bool bAlreadyActive = __super::ApplyShader(); //Pass doesnt matter to DX11Shader

	if(bAlreadyActive)
	{
		//Calling ApplyShader() while already active is double-setting per-frame
		//TODO:Skip if already active? May cause issues if FinishShader() isn't called at end of frame
		//		Possible to call IShader::GetActiveShader().FinishShader() in renderer->SceneEnd();
	}

	//Set the desired pass
	bool bPassSet = SetPass(_iPass);

	//Get Pass Info
	TShaderPass tPass = m_vecPasses[m_iActivePass];

	//Fill the per frame cbuffer
	TCBufferScenePerFrame tCBPerFrame;
	tCBPerFrame.tSun.matSunVP = m_pSunLight->GetViewProjMatrix().Transpose();
	tCBPerFrame.tSun.vec3SunPos = m_pSunLight->GetPosition();
	tCBPerFrame.tSun.vec3SunDir = m_pSunLight->GetLook();
	tCBPerFrame.tSun.tSunProperties = m_pSunLight->GetDefinition();

	//Memcpy
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	HRESULT hrMapped = m_pRenderer->GetDeviceContext()->Map(m_pCBuffers[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if(SUCCEEDED(hrMapped))
	{
		memcpy_s(MappedResource.pData, sizeof(TCBufferScenePerFrame), &tCBPerFrame, sizeof(TCBufferScenePerFrame));
		m_pRenderer->GetDeviceContext()->Unmap(m_pCBuffers[0], 0);
	}

	//Apply the per-frame cbuffer data to register(b1)
	int iCbSlot = (int)EDefaultShaderBindings::CB_PERFRAME;
	m_pRenderer->GetDeviceContext()->VSSetConstantBuffers(iCbSlot, 1, &m_pCBuffers[0]);
	m_pRenderer->GetDeviceContext()->PSSetConstantBuffers(iCbSlot, 1, &m_pCBuffers[0]);

	//TODO: fix this
	return(false);
}

bool
CDefaultShader::SetPass(int _iPass)
{
	//DX11Shader will bind the pass shaders for us
	bool bSuccessful = __super::SetPass(_iPass); //Fails if out of bounds or shader not applied

	if(bSuccessful)
	{
		//Last pass cleanup
		if(_iPass != m_iActivePass) FinishShader();
		sm_pActiveShader = this; //Rebind as FinishShader() clears this naturally

		//Local to us, store as pass is now active
		m_iActivePass = _iPass;

		//Pass actions
		ID3D11RenderTargetView* pNullView = nullptr; //Cannot be written in the switch
		switch(m_iActivePass)
		{
			//Shadow map pass
		case 0:
			//Assume m_pShadowMapSRV is not bound to t1, calling unbind here is a potential waste of cycles if other shaders have been running

			//Clear depth
			m_pRenderer->GetDeviceContext()->ClearDepthStencilView(m_pShadowMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

			//Set a null render target as we are not writing color, only depth
			m_pRenderer->GetDeviceContext()->OMSetRenderTargets(1, &pNullView, m_pShadowMapDSV);

			//Set the raster state
			m_pRenderer->GetDeviceContext()->RSSetState(m_prsShadow);

			//Apply sun camera and force it to update
			m_pSunLight->SetAsActiveCamera();
			m_pSunLight->Process(); //Force update as this sets the viewports
			break;

			//Normal render pass
		case 1:
			//Bind scene camera
			m_pSceneCamera->SetAsActiveCamera();

			//Copy the shadow depth map texture to the shader
			if(m_pShadowMapSRV) m_pRenderer->GetDeviceContext()->PSSetShaderResources((int)EDefaultShaderBindings::TX_SHADOWMAP, 1, &m_pShadowMapSRV);
			break;

			//Error state
		default:
			//Error
			break;
		}
	}

	return(bSuccessful);
}

void
CDefaultShader::FinishShader()
{
	//CDX11Shader::FinishShader()
	__super::FinishShader();

	//There is a switch here for if a shader is interrupted mid frame, for whatever reason that may be, just covering my bases
	ID3D11ShaderResourceView* nullRes = nullptr; //Cannot be written in the switch
	switch(m_iActivePass)
	{
		//Shadow map pass
	case 0:
		//Reset the renderer as the pass0 messes with the targets
		m_pRenderer->RebindSwapChainTarget(true);
		break;

		//Normal render pass
	case 1:
		//Unbind slot 1 (m_pShadowMapSRV) for the next frame so pass0 can run normally
		m_pRenderer->GetDeviceContext()->PSSetShaderResources((int)EDefaultShaderBindings::TX_SHADOWMAP, 1, &nullRes);
		break;

		//Error state
	default:
		//Error
		break;
	}

	//Technically have no active pass currently, prevents FinishShader from repeating actions already executed
	m_iActivePass = -1;
}

bool
CDefaultShader::Predraw(const IMesh* _pMesh, const float4x4* _ptWorldMatrix, bool _bInstanced)
{
	bool bSuccessful = false;
	bool bIsActiveShader = (sm_pActiveShader == this);
	bool bRendererReady = m_pRenderer && m_pRenderer->GetDeviceContext() && m_pRenderer->IsSceneActive();
	bool bValidParams = _pMesh; //nullptr check
	bool bShouldDraw = true;

	bShouldDraw = m_iActivePass || (!m_iActivePass && _pMesh->GetMaterial().bCastShadow);

	if(bIsActiveShader && bRendererReady && bValidParams && bShouldDraw)
	{
		//TODO: Use an enum for swapping here instead of this garbage
		//Calling to super (CDX11SHADER) ignores a lot of sets and only affects the GPU bindings
		m_pRenderer->GetDeviceContext()->VSSetShader(m_vecPasses[m_iActivePass + (_bInstanced ? 2 : 0)].pVertexShader, nullptr, 0);
		m_pRenderer->GetDeviceContext()->IASetInputLayout(m_vecPasses[m_iActivePass + (_bInstanced ? 2 : 0)].pVertexLayout);

		//Build the cbuffer
		TCBufferScenePerObject tCBPerObject;

		//_ptWorldMatrix can sometimes be null when drawing instanced, replace with identity matrix if null
		tCBPerObject.matWorld = _ptWorldMatrix ? _ptWorldMatrix->Transpose() : float4x4::Identity();
		tCBPerObject.bRenderUnlit = false; //TODO: Find a home for this to work properly. Consider removing Predraw from CMesh

		//Fill the per-object cbuffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		HRESULT hrMapped = m_pRenderer->GetDeviceContext()->Map(m_pCBuffers[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		if(SUCCEEDED(hrMapped))
		{
			memcpy_s(MappedResource.pData, sizeof(TCBufferScenePerObject), &tCBPerObject, sizeof(TCBufferScenePerObject));
			m_pRenderer->GetDeviceContext()->Unmap(m_pCBuffers[1], 0);
		}

		//Apply the per-object cbuffer data to register(b2)
		//TODO: Use a better system than this for keeping track of shader bindings, preferably through CRenderer, using a Dictionary maybe
		int iCbSlot = (int)EDefaultShaderBindings::CB_PEROBJECT;
		if(m_vecPasses[m_iActivePass].pVertexShader) m_pRenderer->GetDeviceContext()->VSSetConstantBuffers(iCbSlot, 1, &m_pCBuffers[1]);
		if(m_vecPasses[m_iActivePass].pPixelShader) m_pRenderer->GetDeviceContext()->PSSetConstantBuffers(iCbSlot, 1, &m_pCBuffers[1]);

		//Textured normal render pass (pass1)
		if(m_iActivePass == 1)
		{
			ID3D11ShaderResourceView* pSRVs[4] = { nullptr };

			auto pErrorTex = AssetLoaded(m_pErrorTex) ? m_pErrorTex->GetSRV() : nullptr;
			auto pBlackTex = AssetLoaded(m_pBlackTex) ? m_pBlackTex->GetSRV() : nullptr;
			auto pWhiteTex = AssetLoaded(m_pWhiteTex) ? m_pWhiteTex->GetSRV() : nullptr;

			//Bind valid material parts to the SRV
			TMaterial tMat = _pMesh->GetMaterial();
			pSRVs[0] = AssetLoaded(tMat.pDiffuseTex)	? tMat.pDiffuseTex->GetSRV()	: pErrorTex;
			pSRVs[1] = AssetLoaded(tMat.pNormalTex)		? tMat.pNormalTex->GetSRV()		: pBlackTex;
			pSRVs[2] = AssetLoaded(tMat.pSpecularTex)	? tMat.pSpecularTex->GetSRV()	: pBlackTex;
			pSRVs[3] = AssetLoaded(tMat.pAOTex)			? tMat.pAOTex->GetSRV()			: pWhiteTex;

			//Bind
			m_pRenderer->GetDeviceContext()->PSSetShaderResources(ShaderGlobals::TX_DIFFUSE, 4, pSRVs);
		}

		//Reset to normal pass
		//TODO: Calling this here causes shader mismatch and timing issues where the gpu has things in the wrong slots
		//TODO: Consider storing information on when to change this internally
		//		Perf boost maybe? But one would assume most meshes are not instanced so this call would be rare
		//if(_bInstanced) m_pRenderer->GetDeviceContext()->VSSetShader(m_vecPasses[m_iActivePass].pVertexShader, nullptr, 0);

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

void
CDefaultShader::SetDefaultTextures(CTexture* _pError, CTexture* _pBlack, CTexture* _pWhite)
{
	m_pErrorTex = _pError;
	m_pBlackTex = _pBlack ? _pBlack : _pError;
	m_pWhiteTex = _pWhite ? _pWhite : _pError;
}

void
CDefaultShader::CalculateSceneShadow(std::vector<CEntity3D*> _vecpEntities)
{
	//TODO: Swap out this with quad tree for speed instead of checking every entity
	//TODO: Correctly build ortho based on camera/sun rotation

	float fFOV = m_pSceneCamera->GetFOV(false);
	float3 vec3SceneMin = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 vec3SceneMax = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	const DirectX::BoundingFrustum& tCameraFrustum = m_pSceneCamera->GetBoundingFrustum();

	//For each entity provided to us
	for(unsigned int i = 0; i < _vecpEntities.size(); ++i)
	{
		if(!_vecpEntities[i]->IsVisible() || !_vecpEntities[i]->GetCastShadows()) continue;

		//Use Bounding Sphere, rougher calc but it's a lot faster than using the bounding box for frustum checks
		//Replace this with quad tree instead of testing every single entity, just test against the quadtree
		DirectX::BoundingSphere tSphere = _vecpEntities[i]->GetBoundingSphere();

		//Calculate if we are looking at the entity, this saves computation time in a lot of cases
		float3 vec3Entity = tSphere.Center;
		float3 vec3Look = m_pSceneCamera->GetLook();
		float3 vec3Camera = m_pSceneCamera->GetPosition() - vec3Look; //Offset backwards to capture a little bit more
		float3 vec3Diff = (vec3Entity - vec3Camera);

		//Skip if it doesn't take up enough screen space, saves some processing time
		if(tSphere.Radius < (vec3Diff.Mag() * 0.0125f)) continue;

		float fAngleBetween = acosf(vec3Look.Dot(vec3Diff.Normalize()));

		//Only do a BB collision if we're looking in that direction, otherwise skip
		if(fAngleBetween <= fFOV * 0.5f)
		{
			//Bounding Sphere, Faster than box due to GetCorners() doing transforms. Rougher but it saves a lot of computational time
			if(tCameraFrustum.Contains(tSphere))
			{
				//scene bb min
				vec3SceneMin.x = min(vec3SceneMin.x, tSphere.Center.x - tSphere.Radius);
				vec3SceneMin.y = min(vec3SceneMin.y, tSphere.Center.y - tSphere.Radius);
				vec3SceneMin.z = min(vec3SceneMin.z, tSphere.Center.z - tSphere.Radius);

				//scene bb max
				vec3SceneMax.x = max(vec3SceneMax.x, tSphere.Center.x + tSphere.Radius);
				vec3SceneMax.y = max(vec3SceneMax.y, tSphere.Center.y + tSphere.Radius);
				vec3SceneMax.z = max(vec3SceneMax.z, tSphere.Center.z + tSphere.Radius);
			}
		}
	}

	//Calculate scene box and max length
	float3 vec3Center = (vec3SceneMin + vec3SceneMax) * 0.5f;
	float3 vec3Extents = (vec3SceneMax - vec3SceneMin) * 0.5f;
	float fCornerLength = vec3Extents.Mag();

	//Position camera from center of scene out to the edge of the sphere
	m_pSunLight->SetPosition(vec3Center - (m_pSunLight->GetLook() * fCornerLength));

	//Loose fit for shadowmap
	fCornerLength *= 2.5f;
	m_pSunLight->SetOrthographicMatrix(fCornerLength, fCornerLength, 1.0f, fCornerLength);
}

CLight*
CDefaultShader::GetSun() const
{
	return(m_pSunLight);
}
