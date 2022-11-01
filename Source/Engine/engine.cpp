//Library Includes
#include <ctime>

//Local Includes
#include "common.h"
#include "windowcreation.h"
#include "clock.h"
#include "inputmanager.h"
#include "renderer.h"
#include "logmanager.h"

//This Include
#include "engine.h"

//Static Variables
CEngine* CEngine::sm_pSelf = nullptr;

//Implementation
CEngine::CEngine()
	: m_pRenderer(nullptr)
	, m_pClock(nullptr)
	, m_hWND(NULL)
{
	ZeroMemory(&m_tWindowData, sizeof(TWindowData));
}

CEngine::~CEngine()
{
	SafeDelete(m_pRenderer);
	SafeDelete(m_pClock);

	//Release the input manager
	CInputManager::DestroyInstance();
	CLogManager::DestroyInstance();
}

CEngine&
CEngine::GetInstance()
{
	if(!sm_pSelf) sm_pSelf = new CEngine();
	return(*sm_pSelf);
}

void
CEngine::DestroyInstance()
{
	if(sm_pSelf) SafeDelete(sm_pSelf);
}

bool
CEngine::Initialize(TWindowData _tWindowData)
{
	//Enable run-time memory check for debug builds.
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //mem check
	CLogManager::GetInstance().Initialize(true, true);
#else
	CLogManager::GetInstance().Initialize(false);
#endif

	//Safety Check
	bool bSafe = true;

	//Seed the random number generator
	srand(static_cast<unsigned int>(time(NULL)));

	//Create the window, make sure it exists
	m_hWND = CreateAndRegisterWindow(_tWindowData.hInstance,
									 _tWindowData.iWidth,
									 _tWindowData.iHeight,
									 _tWindowData.wstrTitle,
									 _tWindowData.bFullscreen);

	//Window check
	bSafe = bSafe && (m_hWND != NULL);

	//Create the engine clock and init it
	m_pClock = new CClock();
	if(m_pClock) m_pClock->Initialize();
	bSafe = bSafe && (m_pClock != nullptr);

	//Create the renderer based on the selected DX/OpenGL version
	switch(_tWindowData.eRendererVersion)
	{
		case RENDERER_DX11:
			m_pRenderer = new CRenderer();
			break;

		default:
			DebugBreak(); //TODO: Fix this switch
			bSafe = false;
			break;
	}

	//Renderer Check
	bSafe = bSafe && (m_pRenderer != nullptr);

	//Init renderer if created
	if(m_pRenderer) bSafe = bSafe && m_pRenderer->Initialize(m_hWND, _tWindowData.iWidth, _tWindowData.iHeight, !_tWindowData.bFullscreen);

	//Init the input manager
	bSafe = bSafe && CInputManager::GetInstance().Initialize(false);

	//Return true if everything initialised properly
	return(bSafe);
}

bool
CEngine::InitialiseFromFile()
{
	//TODO: Preload a tWindowData from a file such as XML or something of the like...
	return false;
}

int
CEngine::GameLoop(bool _fpGameFunc(float _fDeltaTick), void _fpDraw())
{
	//Storage bool for the game update function
	//TODO: Error check bGameOk in the engine game loop
	bool bGameOk = true;

	//Make sure msg is initialized with empty values
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//If the window created successfully
	if(m_hWND)
	{
		//Start the game clock
		m_pClock->Start();

		//Main loop
		while(msg.message != WM_QUIT && bGameOk)
		{
			//Message check
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				//Deal with incoming windows messages
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				//Update the input
				CInputManager::GetInstance().Process();

				//Process the game logic
				bGameOk = (bGameOk && _fpGameFunc(m_pClock->GetDeltaTime()));

				//Only draw if the game is running okay
				if(bGameOk)
				{
					//Prepare the renderer
					m_pRenderer->SceneBegin();

					//Draw the game
					_fpDraw();

					//Present the renderer
					m_pRenderer->SceneEnd();
				}

				//Process the clock
				m_pClock->Tick();

				//Sleep to lower CPU usage
				Sleep(1);
			}
		}

		//Stop the game clock
		m_pClock->Stop();
	}
	else
	{
		//Window failed to create
	}

	//Exit with code in msg.wParam
	return(static_cast<int>(msg.wParam));
}

CRenderer*
CEngine::GetRenderer()
{
	return(m_pRenderer);
}

CClock*
CEngine::GetClock()
{
	return(m_pClock);
}

void
CEngine::WinProc(unsigned int _uiMessage, WPARAM _wParam, LPARAM _lParam)
{
	CInputManager::GetInstance().ProcessWindowsMsg(_uiMessage, _wParam, _lParam);
	if(m_pRenderer) m_pRenderer->ProcessWindowsMsg(_uiMessage, _wParam, _lParam);
}
