//Library Includes
#include <windows.h>

//Local Includes
#include "xinputcontroller.h"

//This Include
#include "inputmanager.h"

//Static Variables
CInputManager* CInputManager::sm_pSelf = nullptr;

//Implementation
CInputManager::CInputManager()
	: m_eLastActiveDevice(INPUT_DEV_KEYBOARD)
	, m_vec3MousePosition(0.0f, 0.0f, 0.0f)
{
	//Constructor
	for(int i = 0; i < INPUT_MAX_CONTROLLERS; ++i) m_pControllers[i] = nullptr;
	for(int i = 0; i < MBUTTONS_MAX; ++i) m_bMButtonStates[i] = false;
	ReleaseKeys();
}

CInputManager::~CInputManager()
{
	//Destructor
	for(int i = 0; i < INPUT_MAX_CONTROLLERS; ++i) SafeDelete(m_pControllers[i]);
}

CInputManager&
CInputManager::GetInstance()
{
	//Singleton creation
	if(!sm_pSelf) sm_pSelf = new CInputManager();
	return (*sm_pSelf);
}

void
CInputManager::DestroyInstance()
{
	//Singleton destruction
	SafeDelete(sm_pSelf);
}

bool
CInputManager::IsValid()
{
	return(sm_pSelf != nullptr);
}

bool
CInputManager::Initialize(bool _bSupportController)
{
	bool bIsSuccessful = true;

	if(_bSupportController)
	{
		//Create the controller classes
		for(int i = 0; i < INPUT_MAX_CONTROLLERS; ++i)
		{
			m_pControllers[i] = new CXInputController();
			EControllerStatus eStatus = m_pControllers[i]->Initialize(this, i);

			if(eStatus != CONTROLLER_STATUS_CONNECTED && eStatus != CONTROLLER_STATUS_DISCONNECTED)
			{
				SafeDelete(m_pControllers[i]);
				bIsSuccessful = false;
			}
			else
			{
				//Controller created successfully
				bIsSuccessful = (true && bIsSuccessful);
			}
		}
	}

	return(bIsSuccessful);
}

void
CInputManager::Process()
{
	for(int i = 0; i < INPUT_MAX_CONTROLLERS; ++i) if(m_pControllers[i]) m_pControllers[i]->Process();
}

void
CInputManager::SetMousePosition(int _iX, int _iY, int _iZ)
{
	m_vec3MousePosition = float3(static_cast<float>(_iX), static_cast<float>(_iY), static_cast<float>(_iZ));
}

void
CInputManager::SetMouseButton(EMouseButtons _eButton, bool _bIsPressed)
{
	m_bMButtonStates[_eButton] = _bIsPressed;
}

void
CInputManager::ClearScrollWheel()
{
	m_vec3MousePosition.z = 0.0f;
}

float3
CInputManager::GetMousePosition() const
{
	return (m_vec3MousePosition);
}

void
CInputManager::SetKeyboardInput(WPARAM _vkKey, bool _bIsPressed)
{
	m_eLastActiveDevice = INPUT_DEV_KEYBOARD;
	m_bKeyStates[_vkKey] = _bIsPressed;
}

bool
CInputManager::IsPressed(WPARAM _vkKey) const
{
	return(m_bKeyStates[_vkKey]);
}

bool
CInputManager::IsPressed(EMouseButtons _eButton) const
{
	return(m_bMButtonStates[_eButton]);
}

void
CInputManager::ReleaseKeys()
{
	for(int i = 0; i < 256; ++i) m_bKeyStates[i] = false;
}

void
CInputManager::SetLastActiveDevice(const EInputDevices& _keDevice)
{
	m_eLastActiveDevice = _keDevice;
}

EInputDevices
CInputManager::GetLastActiveDevice() const
{
	return (m_eLastActiveDevice);
}

CXInputController*
CInputManager::GetController(const EInputDevices& _keControllerID) const
{
	CXInputController* pController = nullptr;
	if((_keControllerID < INPUT_MAX_CONTROLLERS) && (_keControllerID > INPUT_INVALID_DEV)) pController = m_pControllers[_keControllerID];
	else if(m_eLastActiveDevice != INPUT_DEV_KEYBOARD) pController = m_pControllers[m_eLastActiveDevice];
	return (pController);
}

bool
CInputManager::ProcessWindowsMsg(unsigned int _uiMessage, WPARAM _wParam, LPARAM _lParam)
{
	bool bHandled = true;

	if(_uiMessage == WM_KEYDOWN || _uiMessage == WM_KEYUP)
	{
		SetLastActiveDevice(INPUT_DEV_KEYBOARD);
		SetKeyboardInput(_wParam, (_uiMessage == WM_KEYDOWN));
	}
	else if(_uiMessage == WM_LBUTTONDOWN || _uiMessage == WM_LBUTTONUP)
	{
		//Left Mouse Button
		SetMouseButton(MOUSEBUTTON_LEFT, (_uiMessage == WM_LBUTTONDOWN));
	}
	else if(_uiMessage == WM_RBUTTONDOWN || _uiMessage == WM_RBUTTONUP)
	{
		//Right Mouse Button
		SetMouseButton(MOUSEBUTTON_RIGHT, (_uiMessage == WM_RBUTTONDOWN));
	}
	else if(_uiMessage == WM_MBUTTONDOWN || _uiMessage == WM_MBUTTONUP)
	{
		//Middle Mouse Button
		SetMouseButton(MOUSEBUTTON_MIDDLE, (_uiMessage == WM_MBUTTONDOWN));
	}
	else if(_uiMessage == WM_MOUSEMOVE)
	{
		//Mouse Position
		//TODO: inputmanager.cpp: LOWORD, HIWORD may return incorrect results; see: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
		float2 tvec2Position = float2(LOWORD(_lParam), HIWORD(_lParam));
		SetMousePosition(static_cast<int>(tvec2Position.x), static_cast<int>(tvec2Position.y));
	}
	else if(_uiMessage == WM_MOUSEWHEEL)
	{
		//Scroll Wheel
		float3 tvec3Position = GetMousePosition();
		SetMousePosition(static_cast<int>(tvec3Position.x), static_cast<int>(tvec3Position.y), GET_WHEEL_DELTA_WPARAM(_wParam));
	}
	else if(_uiMessage == WM_ACTIVATE)
	{
		if(LOWORD(_wParam) == WA_INACTIVE) ReleaseKeys();
	}
	else
	{
		bHandled = false;
	}

	return(bHandled);
}