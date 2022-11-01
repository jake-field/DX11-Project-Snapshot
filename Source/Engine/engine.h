#pragma once
#ifndef __ENGINE_H__
#define __ENGINE_H__

#pragma comment(lib, "d3d11.lib")

//#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "..\\lib\\assimp-vc142-mtd.lib") //assimp for asset loading
//#pragma comment(lib, "xaudio2.lib")
//#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dcompiler.lib")
#endif //_DEBUG

//Library Includes
#include <windows.h>

//Types
enum ERendererVersion
{
	//RENDERER_DX10,
	RENDERER_DX11,
	//RENDERER_DX12
	//RENDERER_OPENGL
	//RENDERER_VULKAN
};

struct TWindowData
{
	HINSTANCE hInstance;
	ERendererVersion eRendererVersion;
	int iWidth;
	int iHeight;
	bool bFullscreen;
	LPCWSTR wstrTitle;
};

//Prototypes
class CClock;
class CRenderer;
class CEngine
{
	//Member Functions
public:
	static CEngine& GetInstance();
	static void DestroyInstance();

	bool Initialize(TWindowData _tWindowData);
	bool InitialiseFromFile(); //Attempt to load settings from a file?
	int GameLoop(bool _fpProcess(float _fDeltaTick), void _fpDraw()); //Loops _pGameFunc until something happens

	CRenderer* GetRenderer();
	CClock* GetClock();

	void WinProc(unsigned int _uiMessage, WPARAM _wParam, LPARAM _lParam);

private:
	CEngine();
	~CEngine();

	//Member Variables
protected:
	static CEngine* sm_pSelf;

	CRenderer* m_pRenderer;
	CClock* m_pClock;

	TWindowData m_tWindowData;
	HWND m_hWND;

};

#endif //__ENGINE_H__