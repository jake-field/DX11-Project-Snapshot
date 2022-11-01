#pragma once
#ifndef __XINPUT_CONTROLLER_H__
#define __XINPUT_CONTROLLER_H__

//Library Includes
#include <xinput.h>

//Local Includes
#include "types.h"

//Preprocessor
#define XINPUT_MAX_CONTROLLERS 4 //TODO: This needs to be revised to match new specs of the BT update which increased this to 8
#define XINPUT_DEFAULT_THUMB_DZ 0.2f
#define XINPUT_DEFAULT_SENSITIVITY 1.0f

//Types
typedef enum EControllerButtons
{
	CONTROLLER_DPAD_UP = 0x00000001,
	CONTROLLER_DPAD_DOWN = 0x00000002,
	CONTROLLER_DPAD_LEFT = 0x00000004,
	CONTROLLER_DPAD_RIGHT = 0x00000008,
	CONTROLLER_START = 0x00000010,
	CONTROLLER_BACK = 0x00000020,
	CONTROLLER_LEFT_THUMB = 0x00000040,
	CONTROLLER_RIGHT_THUMB = 0x00000080,
	CONTROLLER_LEFT_SHOULDER = 0x0100,
	CONTROLLER_RIGHT_SHOULDER = 0x0200,
	CONTROLLER_A = 0x1000,
	CONTROLLER_B = 0x2000,
	CONTROLLER_X = 0x4000,
	CONTROLLER_Y = 0x8000,
	CONTROLLER_MAX_BUTTONS = 14,
} EControllerButtons;

typedef enum EControllerStatus
{
	CONTROLLER_STATUS_INVALID_ID,
	CONTROLLER_STATUS_INVALID_MANAGER,
	CONTROLLER_STATUS_DISCONNECTED,
	CONTROLLER_STATUS_CONNECTED
} EControllerStatus;

typedef enum EControllerBattery
{
	CONTROLLER_BATTERY_EMPTY,
	CONTROLLER_BATTERY_LOW,
	CONTROLLER_BATTERY_MEDIUM,
	CONTROLLER_BATTERY_FULL
} EControllerBattery;

typedef enum EControllerAimSettings
{
	CONTROLLER_AIM_LINEAR,
	CONTROLLER_AIM_CUBIC,
} EControllerAimSettings;

//Prototypes
class CInputManager;
class CXInputController
{
	//Member Functions
public:
	CXInputController();
	~CXInputController();

	EControllerStatus Initialize(CInputManager* _pInputManager, unsigned int _uiController);
	void Process();

	unsigned int GetID() const;
	unsigned short GetButtonStates() const;
	float2 GetLeftThumbPosition() const;
	float2 GetRightThumbPosition() const;
	float GetLeftTriggerPosition() const;
	float GetRightTriggerPosition() const;

	void SetLookYInverted(bool _bInvert);

	void SetAimSensitivity(float _fSensitivity);
	float GetAimSensitivity() const;

	void SetAimSetting(EControllerAimSettings _eSetting);
	EControllerAimSettings GetAimSetting() const;

	void SetLeftThumbStickDeadZone(float _fDeadZone);
	void SetRightThumbStickDeadZone(float _fDeadZone);
	float GetLeftThumbStickDeadZone() const;
	float GetRightThumbStickDeadZone() const;

	void SetRumble(float _fLeft, float _fRight);
	float GetRumbleLeft() const;
	float GetRumbleRight() const;

	EControllerBattery GetBatteryLevel() const;

	bool IsConnected() const;
	bool IsWireless() const;
	bool IsLookYInverted() const;
	bool IsPressed(unsigned short _usButtons) const;

protected:
	void RetrieveState();
	void ReadBatteryLevel();
	void ReadTriggers(XINPUT_STATE& _rState);
	void ReadAnalogueSticks(XINPUT_STATE& _rState);
	void ReadButtons(XINPUT_STATE& _rState);

private:
	CXInputController(const CXInputController& _kr);
	CXInputController& operator= (const CXInputController& _kr);

	//Member Variables
protected:
	CInputManager* m_pInputManager;
	unsigned int m_uiController;

	bool m_bIsConnected;
	bool m_bIsWireless;
	bool m_bIsLookYInverted;

	EControllerBattery m_eBatteryLevel;
	EControllerAimSettings m_eAimSetting;

	float m_fAimSensitivity;
	float m_fLeftRumble;
	float m_fRightRumble;
	float m_fLeftTrigger;
	float m_fRightTrigger;
	float m_fLeftThumbStickX;
	float m_fLeftThumbStickY;
	float m_fRightThumbStickX;
	float m_fRightThumbStickY;
	float m_fLeftThumbStickDeadZone;
	float m_fRightThumbStickDeadZone;

	unsigned short m_usButtonStates;

};

#endif //__XINPUT_CONTROLLER_H__