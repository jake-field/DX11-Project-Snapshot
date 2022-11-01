//Library Includes
#include <iostream>
#include <string>

//This Include
#include "consolewindow.h"
#include "engine.h"

//Static Variables
CConsoleWindow* CConsoleWindow::sm_pSelf = nullptr;

//Implementation
CConsoleWindow::CConsoleWindow()
	: m_bInitialised(false)
	, m_eState(EConsoleState::CONSOLE_INACTIVE)
{
	//Constructor
}

CConsoleWindow::~CConsoleWindow()
{
	//Free the console if we created it
	m_eState = EConsoleState::CONSOLE_INACTIVE;
	if(m_bInitialised) FreeConsole();
}

CConsoleWindow&
CConsoleWindow::GetInstance()
{
	//Singleton creation
	if(!sm_pSelf) sm_pSelf = new CConsoleWindow();
	return(*sm_pSelf);
}

void
CConsoleWindow::DestroyInstance()
{
	//Singleton destruction
	if(sm_pSelf)
	{
		delete sm_pSelf;
		sm_pSelf = nullptr;
	}
	else
	{
		//Already Destroyed
	}
}

bool
CConsoleWindow::Initialize()
{
	bool bSuccess = false;

	//True if the console was created
	bSuccess = (AllocConsole() == TRUE);

	if(bSuccess)
	{
		//Console allocated
		m_bInitialised = true;

		//Redirect all cin/cout to our new console
		FILE tNotUsed;
		FILE* ptNotUsed = &tNotUsed;
		freopen_s(&ptNotUsed, "CONIN$", "r", stdin); //Allow cin
		freopen_s(&ptNotUsed, "CONOUT$", "w", stdout); //Allow cout

		//Set up the shutdown handlers
		SetConsoleCtrlHandler(NULL, TRUE);
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandlerRoutine, TRUE);

		//Give the console a meaningfull title (EXE name)
		char strFile[MAX_PATH];
		GetModuleFileNameA(NULL, strFile, MAX_PATH);

		std::string strTitle = "title ";
		strTitle += std::string(strFile).substr(strrchr(strFile, '\\') - strFile + 1);
		strTitle += " - Debug Console";

		//Apply the title
		ExecuteCommand(strTitle.c_str());

		//Console State
		m_eState = EConsoleState::CONSOLE_OPEN;
	}
	else
	{
		//Failed to allocate a new console
	}

	return(bSuccess);
}

void
CConsoleWindow::ExecuteCommand(const char* _pcstr)
{
	//If console is running, execute console command
	if(m_bInitialised) system(_pcstr);
}

void
CConsoleWindow::Write(const char* _pcstr)
{
	//If console is running, print message
	if(m_bInitialised) std::cout << _pcstr;
}

EConsoleState
CConsoleWindow::GetConsoleState()
{
	EConsoleState eState = EConsoleState::CONSOLE_INACTIVE;
	if(sm_pSelf != nullptr) eState = sm_pSelf->m_eState;
	return(eState);
}

BOOL WINAPI
CConsoleWindow::ConsoleHandlerRoutine(DWORD _dwCtrlType)
{
	//This function is run on a seperate thread...
	//Event switch
	switch(_dwCtrlType)
	{
		//The following events will cause Visual Studio to force a breakpoint in debug mode
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
			break;

		//We have ~10 seconds to clean up, by then we get terminated by Windows 7 or higher.
		//Assume running windows 7 and accept shutdown.
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			CConsoleWindow::GetInstance().m_eState = EConsoleState::CONSOLE_SHUTDOWN;
			CConsoleWindow::GetInstance().Write("\nForcing program close...\n");
			CEngine::DestroyInstance(); //Should suffice for clean-shutdown
			break;

		default:
			//Not handled
			break;
	}

	//Handled
	return(TRUE);
}