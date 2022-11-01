#pragma once
#ifndef __WINDOW_CREATION_H__
#define __WINDOW_CREATION_H__

//Library Includes
#include <windows.h>

//Local Includes
#include "engine.h"

//Preprocessor
constexpr auto WINDOW_CLASS_NAME = L"ENGINE";

//Implementation
LRESULT CALLBACK MainWindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam)
{
	//Filter messages
	if(_msg == WM_DESTROY)
	{
		//Quit
		PostQuitMessage(0);
	}
	else if(_msg == WM_GETMINMAXINFO) //TODO: Remove this section if we are running fixed resolution options
	{
		//Constant variables
		const int kiBorderWidth = 16;
		const int kiBorderHeight = 38;
		const int kiMinimumWidth = 800;
		const int kiMinimumHeight = 600; //TODO: Adjust these min res constraints

		//Make sure the window doesn't go too small (Client area)
		reinterpret_cast<MINMAXINFO*>(_lparam)->ptMinTrackSize.x = (kiMinimumWidth + kiBorderWidth);
		reinterpret_cast<MINMAXINFO*>(_lparam)->ptMinTrackSize.y = (kiMinimumHeight + kiBorderHeight);
	}

	//Send off the message to the engine so that ALL messages can be checked
	CEngine::GetInstance().WinProc(_msg, _wparam, _lparam);

	//Return message back to Windows
	return (DefWindowProc(_hwnd, _msg, _wparam, _lparam));
}

HWND CreateAndRegisterWindow(HINSTANCE _hInstance, int _iWidth, int _iHeight, LPCWSTR _pcTitle, bool _bIsFullscreen)
{
	//Return variable
	HWND hWnd = NULL;

	//Window Class Definition
	WNDCLASS tWindowDefinition;
	tWindowDefinition.style = (CS_HREDRAW | CS_VREDRAW);
	tWindowDefinition.lpfnWndProc = MainWindowProc;
	tWindowDefinition.cbClsExtra = NULL;
	tWindowDefinition.cbWndExtra = NULL;
	tWindowDefinition.hInstance = _hInstance;
	tWindowDefinition.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	tWindowDefinition.hCursor = LoadCursor(NULL, IDC_ARROW);
	tWindowDefinition.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
	tWindowDefinition.lpszMenuName = NULL;
	tWindowDefinition.lpszClassName = WINDOW_CLASS_NAME;

	//Register window definition
	if(RegisterClass(&tWindowDefinition))
	{
		//Successfully registered, attempt to create the window
		int iWidth = _iWidth;
		int iHeight = _iHeight;

		//Recalculate window size to accommodate title and frame if windowed
		if(!_bIsFullscreen)
		{
			//Recalculate
			RECT tRect = {0, 0, _iWidth, _iHeight};
			AdjustWindowRect(&tRect, WS_OVERLAPPEDWINDOW, FALSE);

			//Store new window size
			iWidth = tRect.right - tRect.left;
			iHeight = tRect.bottom - tRect.top;
		}
		else
		{
			//Don't recalculate window size
		}

		//Create the window and update it
		//TODO: Change WS_OVERLAPPEDWINDOW to one without maximize button if running fixed resolutions
		DWORD dwStyle = ((WS_OVERLAPPEDWINDOW | WS_VISIBLE) * !_bIsFullscreen) | ((WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE) * _bIsFullscreen);
		hWnd = CreateWindow(WINDOW_CLASS_NAME, _pcTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, iWidth, iHeight, NULL, NULL, _hInstance, NULL);
		UpdateWindow(hWnd);
	}
	else
	{
		//Failed to register the class
	}

	return(hWnd);
}

#endif //__WINDOW_CREATION_H__