//Library Includes
#include <Engine\engine.h>

//Local Includes
#include "game.h"

int WINAPI
WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE _hPrevInstance, _In_ LPSTR _lpCmdLine, _In_ int _nCmdShow)
{
	//Prepare the window struct
	TWindowData tWindowData;
	tWindowData.hInstance = _hInstance;
	tWindowData.wstrTitle = L"Demo Game - DX11";
	tWindowData.eRendererVersion = RENDERER_DX11;
	tWindowData.iWidth = 1280; //TODO: Set this via a device enumerator
	tWindowData.iHeight = 960;
	tWindowData.bFullscreen = false;

	//Create the engine
	CEngine& rEngine = CEngine::GetInstance();
	rEngine.Initialize(tWindowData);

	//Create the game
	CGame& rGame = CGame::GetInstance();
	rGame.Initialize();

	//Game loop
	int iReturnCode = rEngine.GameLoop(CGame::_Process, CGame::_Draw);

	//Clean up
	CGame::DestroyInstance();
	CEngine::DestroyInstance();

	return(iReturnCode);
}
