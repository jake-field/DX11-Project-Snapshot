#pragma once
#ifndef __CONSOLEWINDOW_H__
#define __CONSOLEWINDOW_H__

//Library Includes
#include <windows.h>

//Types
enum class EConsoleState
{
	CONSOLE_INACTIVE,
	CONSOLE_OPEN,
	CONSOLE_SHUTDOWN
};

//Prototypes
class CConsoleWindow
{
	//Member Functions
private:
	CConsoleWindow();
	~CConsoleWindow();

public:
	static CConsoleWindow& GetInstance();
	static void DestroyInstance();

	bool Initialize();

	void ExecuteCommand(const char* _pcstr);

	void Write(const char* _pcstr);

	static EConsoleState GetConsoleState();

protected:
	static BOOL WINAPI ConsoleHandlerRoutine(DWORD _dwCtrlType);


	//Member Variables
protected:
	static CConsoleWindow* sm_pSelf; //TODO: allow for soft destruction, in-case multiple classes are using the functionality of this class
	bool m_bInitialised;
	EConsoleState m_eState;
};

#endif //__CONSOLEWINDOW_H__