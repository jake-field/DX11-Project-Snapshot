//Local Include
#include "ishader.h"
#include "camera.h"

//This Include
#include "renderer.h"

//Implementation
CRenderer::CRenderer()
{
	//Constructor
	m_hWnd = nullptr;
	m_iWidth = 0;
	m_iHeight = 0;
	m_bResizing = false;
	m_bSceneActive = false;
	m_bRenderWireframe = false;

	//Default clear color
	m_tClearColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	//DX coms
	m_pSwapChain = nullptr;
	m_pDevice = nullptr;
	m_pDeviceContext = nullptr;
	m_pRenderTarget[0] = nullptr;
	m_pRenderTarget[1] = nullptr;
	m_pDepthStencil = nullptr;
	m_pDepthStencilBuffer = nullptr;
	m_pGlobalCBuffer = nullptr;

	for(int i = 0; i < DefaultSamplerStates::MAX_SS; ++i) m_pSamplerStates[i] = nullptr;
	for(int i = 0; i < DefaultRasterStates::MAX_RS; ++i) m_pRasterStates[i] = nullptr;
	for(int i = 0; i < DefaultBlendStates::MAX_BS; ++i) m_pBlendStates[i] = nullptr;

}

CRenderer::~CRenderer()
{
	//Destructor
	Shutdown();
}

bool CRenderer::Initialize(HWND _hWindow, int _iWidth, int _iHeight, bool _bWindowed)
{
	bool bSuccess = true;

	bSuccess = bSuccess && StartDX11(_hWindow, _iWidth, _iHeight, _bWindowed);
	bSuccess = bSuccess && Resize(_iWidth, _iHeight);

	return(bSuccess); //TODO: Finish Initialise
}

void CRenderer::Shutdown()
{
	if(m_pDeviceContext) m_pDeviceContext->ClearState();

	for(int i = 0; i < DefaultSamplerStates::MAX_SS; ++i) ReleaseCOM(m_pSamplerStates[i]);
	for(int i = 0; i < DefaultRasterStates::MAX_RS; ++i) ReleaseCOM(m_pRasterStates[i]);
	for(int i = 0; i < DefaultBlendStates::MAX_BS; ++i) ReleaseCOM(m_pBlendStates[i]);

	//CBuffer
	ReleaseCOM(m_pGlobalCBuffer);

	ReleaseCOM(m_pDepthStencilBuffer);
	ReleaseCOM(m_pDepthStencil);
	ReleaseCOM(m_pRenderTarget[0]);
	ReleaseCOM(m_pRenderTarget[1]);
	ReleaseCOM(m_pSwapChain);
	ReleaseCOM(m_pDeviceContext);
	ReleaseCOM(m_pDevice);
}

bool CRenderer::Resize(int _iWidth, int _iHeight)
{
	HRESULT hr = S_FALSE;

	//Update size
	m_iWidth = _iWidth;
	m_iHeight = _iHeight;

	if(m_pDevice && m_pDeviceContext && m_pSwapChain)
	{
		//Release old if possible
		ReleaseCOM(m_pDepthStencilBuffer);
		ReleaseCOM(m_pDepthStencil);
		ReleaseCOM(m_pRenderTarget[0]);
		ReleaseCOM(m_pRenderTarget[1]);

		//Resize and create backbuffer
		ID3D11Texture2D* pBackBuffer = NULL;

		hr = m_pSwapChain->ResizeBuffers(2, m_iWidth, m_iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTarget[0]);
		ReleaseCOM(pBackBuffer);

		//Create the depthstencil
		D3D11_TEXTURE2D_DESC tDepthStencilDesc;
		ZeroMemory(&tDepthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
		tDepthStencilDesc.Width = m_iWidth;
		tDepthStencilDesc.Height = m_iHeight;
		tDepthStencilDesc.MipLevels = 1;
		tDepthStencilDesc.ArraySize = 1;
		tDepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		tDepthStencilDesc.SampleDesc.Count = 1;
		tDepthStencilDesc.SampleDesc.Quality = 0;
		tDepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		tDepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		tDepthStencilDesc.CPUAccessFlags = 0;
		tDepthStencilDesc.MiscFlags = 0;

		hr = m_pDevice->CreateTexture2D(&tDepthStencilDesc, 0, &m_pDepthStencilBuffer);
		hr = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencil);

		//Bind the view
		m_pDeviceContext->OMSetRenderTargets(2, m_pRenderTarget, m_pDepthStencil);
	}

	return(SUCCEEDED(hr));
}

bool CRenderer::SceneBegin()
{
	bool bSuccess = false;

	if(!m_bSceneActive && m_pDeviceContext && m_pSwapChain)
	{
		//Get the lock for the scene, this is due to resource allocation
		m_mutexScene.lock();


		bSuccess = true;
		m_bSceneActive = true;
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTarget[0], (float*)(&m_tClearColor));
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);

		float pfBlendFactors[] = {0.0f, 0.0f, 0.0f, 1.0f};
		m_pDeviceContext->OMSetBlendState(m_pBlendStates[DefaultBlendStates::BS_DEFAULT], pfBlendFactors, 0xffffffff);

		m_pDeviceContext->RSSetState(m_bRenderWireframe ? m_pRasterStates[DefaultRasterStates::RS_WIREFRAME] : m_pRasterStates[DefaultRasterStates::RS_SOLID]);
	}
	else if(!m_pDeviceContext)
	{
		//No Device Context
	}
	else if(!m_pSwapChain)
	{
		//No swap chain
	}
	else
	{
		//Scene Active
	}

	return(bSuccess);
}

bool CRenderer::SceneEnd()
{
	bool bSuccess = false;

	if(m_bSceneActive && m_pSwapChain)
	{
		//Call finish shader
		//TODO: Is this required?
		IShader* pShader = IShader::GetActiveShader();
		if(pShader) pShader->FinishShader();

		//Present the final scene
		m_bSceneActive = false;
		bSuccess = SUCCEEDED(m_pSwapChain->Present(0, 0));

		//Due to DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
		m_pDeviceContext->OMSetRenderTargets(2, m_pRenderTarget, m_pDepthStencil);

		//Unlock the scene, allows for resource creation
		m_mutexScene.unlock();
	}
	else if(!m_pSwapChain)
	{
		//No swap chain to present
	}
	else
	{
		//Scene Not Active
	}

	return(bSuccess);
}

bool CRenderer::IsSceneActive()
{
	return(m_pDevice && m_bSceneActive);
}

void CRenderer::SetRenderMode(bool _bWireframe)
{
	m_bRenderWireframe = _bWireframe;
}

bool CRenderer::GetRenderMode() const
{
	return(m_bRenderWireframe);
}

void CRenderer::UpdateGlobalCBuffer(CCamera* _pCamera)
{
	//Fill the cbuffer (per-frame)
	auto& cBufferData = ShaderGlobals::gGlobalCBuffer;
	cBufferData.matView = _pCamera->GetViewMatrix().Transpose();
	cBufferData.matProj = _pCamera->GetProjectionMatrix().Transpose();
	cBufferData.matViewProj = _pCamera->GetViewProjMatrix().Transpose();
	cBufferData.tCamera.vec3EyePos = _pCamera->GetPosition();
	cBufferData.tCamera.vec3EyeLook = _pCamera->GetLook();

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_pDeviceContext->Map(m_pGlobalCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	memcpy_s(MappedResource.pData, sizeof(ShaderGlobals::TCBufferGlobal), &cBufferData, sizeof(ShaderGlobals::TCBufferGlobal));
	m_pDeviceContext->Unmap(m_pGlobalCBuffer, 0);

	//Apply the per-frame cbuffer data
	//TODO: set it across other shaders
	m_pDeviceContext->VSSetConstantBuffers(ShaderGlobals::ERegisters::CB_GlobalCBuffer, 1, &m_pGlobalCBuffer);
	m_pDeviceContext->PSSetConstantBuffers(ShaderGlobals::ERegisters::CB_GlobalCBuffer, 1, &m_pGlobalCBuffer);
	m_pDeviceContext->GSSetConstantBuffers(ShaderGlobals::ERegisters::CB_GlobalCBuffer, 1, &m_pGlobalCBuffer);
}

void CRenderer::RebindSwapChainTarget(bool _bResetRasterState)
{
	m_pDeviceContext->OMSetRenderTargets(2, m_pRenderTarget, m_pDepthStencil);
	if(_bResetRasterState) m_pDeviceContext->RSSetState(m_bRenderWireframe ? m_pRasterStates[DefaultRasterStates::RS_WIREFRAME] : m_pRasterStates[DefaultRasterStates::RS_SOLID]);
}


void CRenderer::SetClearColor(float4 _tColor)
{
	m_tClearColor = _tColor;
}

void CRenderer::SetClearColor(float _fRed, float _fGreen, float _fBlue)
{
	m_tClearColor = float4(_fRed, _fGreen, _fBlue, 1.0f);
}

float4 CRenderer::GetClearColor() const
{
	return(m_tClearColor);
}

float2 CRenderer::GetSize() const
{
	return float2((float)m_iWidth, (float)m_iHeight);
}

ID3D11Buffer* CRenderer::CreateBuffer(UINT _uiBufferType, void* _pData, size_t _uiStructSize, D3D11_USAGE _eBufferUsage)
{
	ID3D11Buffer* pBuffer = nullptr;
	D3D11_BUFFER_DESC tBufferDescription;
	D3D11_SUBRESOURCE_DATA  tResourceData;
	ZeroMemory(&tBufferDescription, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&tResourceData, sizeof(D3D11_SUBRESOURCE_DATA));

	tBufferDescription.Usage = _eBufferUsage;
	tBufferDescription.BindFlags = _uiBufferType;
	tBufferDescription.ByteWidth = _uiStructSize;
	tResourceData.pSysMem = _pData;

	//Open buffer to write access if dynamic
	if(_eBufferUsage == D3D11_USAGE_DYNAMIC) tBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	m_pDevice->CreateBuffer(&tBufferDescription, tResourceData.pSysMem ? &tResourceData : nullptr, &pBuffer);

	return(pBuffer); //if nullptr then failed
}

ID3D11Device* CRenderer::GetDevice() const
{
	return(m_pDevice);
}

ID3D11DeviceContext* CRenderer::GetDeviceContext() const
{
	return(m_pDeviceContext);
}

std::mutex& CRenderer::GetGPUMutex()
{
	return(m_mutexScene);
}

void CRenderer::ProcessWindowsMsg(UINT _msg, WPARAM _wparam, LPARAM _lparam)
{
	bool bWasResizing = m_bResizing;
	static LPARAM _lastValid;

	if(_msg == WM_SIZE)
	{
		if(_lparam) _lastValid = _lparam;
		if(_wparam == SIZE_MAXIMIZED)
		{
			Resize(LOWORD(_lparam), HIWORD(_lparam));
		}
		else if(_wparam == SIZE_RESTORED)
		{
			//Window restored
			if(!m_bResizing)
			{
				//Update Renderer
				Resize(LOWORD(_lparam), HIWORD(_lparam));
			}
			else
			{
				//Don't update while resizing
			}
		}
	}
	else if(_msg == WM_ENTERSIZEMOVE)
	{
		//User is starting the resize
		m_bResizing = true;
	}
	else if(_msg == WM_EXITSIZEMOVE)
	{
		//User finished the resize
		m_bResizing = false;
	}

	//Window was resized
	if(bWasResizing && bWasResizing != m_bResizing && _lastValid != 0)
	{
		Resize(LOWORD(_lastValid), HIWORD(_lastValid));
	}
}

bool CRenderer::StartDX11(HWND _hWindow, int _iWidth, int _iHeight, bool _bWindowed)
{
	HRESULT hr = S_OK;
	m_hWnd = _hWindow;
	m_iWidth = _iWidth;
	m_iHeight = _iHeight;

	//Create swap chain
	DXGI_SWAP_CHAIN_DESC tSwapChainDescription;
	ZeroMemory(&tSwapChainDescription, sizeof(DXGI_SWAP_CHAIN_DESC));
	tSwapChainDescription.BufferCount = 2;
	tSwapChainDescription.BufferDesc.Width = m_iWidth;
	tSwapChainDescription.BufferDesc.Height = m_iHeight;
	tSwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tSwapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
	tSwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
	tSwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	tSwapChainDescription.OutputWindow = m_hWnd;
	tSwapChainDescription.SampleDesc.Count = 1;
	tSwapChainDescription.SampleDesc.Quality = 0;
	tSwapChainDescription.Windowed = _bWindowed;
	tSwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//Ordering of feature levels that Direct3D attempts to create
	const UINT uiFeatureLevels = 2;
	D3D_FEATURE_LEVEL eFeatureLevels[uiFeatureLevels] =
	{
		//D3D_FEATURE_LEVEL_12_1,
		//D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		//D3D_FEATURE_LEVEL_10_1,
		//D3D_FEATURE_LEVEL_10_0,
		//D3D_FEATURE_LEVEL_9_3,
		//D3D_FEATURE_LEVEL_9_1
	};

	UINT uiCreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED; //TODO: Change to multi-threaded if that's the case

#ifdef _DEBUG
//#define D3D_DEBUG_INFO
	uiCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDeviceAndSwapChain(nullptr,
									   D3D_DRIVER_TYPE_HARDWARE,
									   nullptr,
									   uiCreationFlags,
									   eFeatureLevels,
									   uiFeatureLevels,
									   D3D11_SDK_VERSION,
									   &tSwapChainDescription,
									   &m_pSwapChain,
									   &m_pDevice,
									   nullptr,
									   &m_pDeviceContext);

	//Create default states
	for(auto i = 0; i < DefaultRasterStates::MAX_RS; ++i)
	{
		m_pDevice->CreateRasterizerState(&DefaultRasterStates::gRasterStates[i], &m_pRasterStates[i]);
	}

	for(auto i = 0; i < DefaultSamplerStates::MAX_SS; ++i)
	{
		m_pDevice->CreateSamplerState(&DefaultSamplerStates::gSamplerStates[i], &m_pSamplerStates[i]);
	}

	for(auto i = 0; i < DefaultBlendStates::MAX_BS; ++i)
	{
		m_pDevice->CreateBlendState(&DefaultBlendStates::gBlendStates[i], &m_pBlendStates[i]);
	}

	//Push Samplers to the gpu
	m_pDeviceContext->PSSetSamplers(DefaultSamplerStates::START_SLOT, DefaultSamplerStates::MAX_SS, m_pSamplerStates);

	//Create the global cbuffer
	m_pGlobalCBuffer = CreateBuffer(D3D11_BIND_CONSTANT_BUFFER, &ShaderGlobals::gGlobalCBuffer, sizeof(ShaderGlobals::TCBufferGlobal), D3D11_USAGE_DYNAMIC);

	//Return true if success
	return(SUCCEEDED(hr));
}
