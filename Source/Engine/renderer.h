#pragma once
#ifndef __RENDERER_H__
#define __RENDERER_H__

//Library Includes
#include <mutex>

//Local Includes
#include "common.h"
#include "samplerstates.h"
#include "rasterstates.h"
#include "blendstates.h"

//Prototypes
class CCamera;
class CRenderer
{
	//Member Functions
public:
	CRenderer();
	~CRenderer();

	bool Initialize(HWND _hWindow, int _iWidth, int _iHeight, bool _bWindowed);
	void Shutdown();

	bool Resize(int _iWidth, int _iHeight);
	
	bool SceneBegin();
	bool SceneEnd();
	bool IsSceneActive();

	void SetRenderMode(bool _bWireframe);
	bool GetRenderMode() const;

	void UpdateGlobalCBuffer(CCamera* _pCamera);

	void RebindSwapChainTarget(bool _bResetRasterState = true);

	void SetClearColor(float4 _tColor);
	void SetClearColor(float _fRed, float _fGreen, float _fBlue);
	float4 GetClearColor() const;

	float2 GetSize() const;

	ID3D11Buffer* CreateBuffer(UINT _uiBufferType, void* _pData, size_t _uiStructSize, D3D11_USAGE _eBufferUsage = D3D11_USAGE_DEFAULT);

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;

	std::mutex& GetGPUMutex();

	//Process the windows message queue
	void ProcessWindowsMsg(UINT _msg, WPARAM _wparam, LPARAM _lparam);

private:
	bool StartDX11(HWND _hWindow, int _iWidth, int _iHeight, bool _bWindowed);

	//Member Variables
protected:
	HWND m_hWnd;
	bool m_bResizing;
	bool m_bSceneActive;
	bool m_bRenderWireframe;

	int m_iWidth;
	int m_iHeight;

	IDXGISwapChain* m_pSwapChain;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11RenderTargetView* m_pRenderTarget[2];
	ID3D11DepthStencilView* m_pDepthStencil;
	ID3D11Texture2D* m_pDepthStencilBuffer;

	//Default Render States
	ID3D11RasterizerState*	m_pRasterStates[DefaultRasterStates::MAX_RS];
	ID3D11SamplerState*		m_pSamplerStates[DefaultSamplerStates::MAX_SS];
	ID3D11BlendState*		m_pBlendStates[DefaultBlendStates::MAX_BS];

	//Global cbuffer
	ID3D11Buffer* m_pGlobalCBuffer;

	//Used in conjunction with the multi-threaded asset manager to prevent device lockups
	std::mutex m_mutexScene;

	float4 m_tClearColor;
};

#endif //__RENDERER_H__
