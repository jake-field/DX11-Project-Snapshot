#pragma once
#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

//Local Includes
#include "common.h"
#include "dxcommon.h"
#include "eventemitter.hpp"

//TODO: Input manager is a broad name, which means it should include action mappings which should be a dictionary with multiple pairs per action.
//		Consider renaming to input host and splitting off the various input systems

//Types
typedef enum EInputDevices
{
	//TODO: inputmanager.h: Certain controller schemes can net up to 8 active controllers now.
	INPUT_INVALID_DEV = -1,
	INPUT_DEV_CONTROLLER_1,
	INPUT_DEV_CONTROLLER_2,
	INPUT_DEV_CONTROLLER_3,
	INPUT_DEV_CONTROLLER_4,
	INPUT_DEV_KEYBOARD,
	INPUT_MAX_CONTROLLERS = 4,
} EInputDevices;

enum EMouseButtons
{
	MOUSEBUTTON_LEFT,
	MOUSEBUTTON_MIDDLE,
	MOUSEBUTTON_RIGHT,

	MBUTTONS_MAX
};

//Prototypes
class CXInputController;
class IInputEventListener;
class CInputManager: public CEventEmitter<IInputEventListener>
{
	//Member Functions
public:
	static CInputManager& GetInstance();
	static void DestroyInstance();
	static bool IsValid();

	bool Initialize(bool _bSupportController);
	void Process();

	void SetMousePosition(int _iX, int _iY, int _iZ = 0);
	void SetMouseButton(EMouseButtons _eButton, bool _bIsPressed);
	void ClearScrollWheel();
	float3 GetMousePosition() const;

	void SetKeyboardInput(WPARAM _vkKey, bool _bIsPressed);
	bool IsPressed(WPARAM _vkKey) const;
	bool IsPressed(EMouseButtons _eButton) const;
	void ReleaseKeys();

	void SetLastActiveDevice(const EInputDevices& _keDevice);
	EInputDevices GetLastActiveDevice() const;

	CXInputController* GetController(const EInputDevices& _keControllerID = INPUT_DEV_CONTROLLER_1) const;

	bool ProcessWindowsMsg(unsigned int _uiMessage, WPARAM _wParam, LPARAM _lParam);

private:
	CInputManager();
	~CInputManager();

	//Member Variables
protected:
	static CInputManager* sm_pSelf;

	EInputDevices m_eLastActiveDevice;
	float3 m_vec3MousePosition;
	bool m_bMButtonStates[MBUTTONS_MAX];
	bool m_bKeyStates[256];

	CXInputController* m_pControllers[INPUT_MAX_CONTROLLERS];
};

#endif //__INPUTMANAGER_H__