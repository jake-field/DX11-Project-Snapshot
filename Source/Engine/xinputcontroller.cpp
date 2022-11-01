//Local Includes
#include "inputmanager.h"

//This Includes
#include "xinputcontroller.h"

//Implementation
CXInputController::CXInputController()
	: m_pInputManager(0)
	, m_uiController(0)
	, m_bIsConnected(false)
	, m_bIsWireless(false)
	, m_bIsLookYInverted(false)
	, m_eBatteryLevel(CONTROLLER_BATTERY_FULL)
	, m_eAimSetting(CONTROLLER_AIM_LINEAR)
	, m_fAimSensitivity(XINPUT_DEFAULT_SENSITIVITY)
	, m_fLeftRumble(0.0f)
	, m_fRightRumble(0.0f)
	, m_fLeftTrigger(0.0f)
	, m_fRightTrigger(0.0f)
	, m_fLeftThumbStickX(0.0f)
	, m_fLeftThumbStickY(0.0f)
	, m_fRightThumbStickX(0.0f)
	, m_fRightThumbStickY(0.0f)
	, m_fLeftThumbStickDeadZone(XINPUT_DEFAULT_THUMB_DZ)
	, m_fRightThumbStickDeadZone(XINPUT_DEFAULT_THUMB_DZ)
	, m_usButtonStates(0)
{
	//Constructor
}

CXInputController::~CXInputController()
{
	//Destructor
}

EControllerStatus
CXInputController::Initialize(CInputManager* _pInputManager, unsigned int _uiController)
{
	//Return status
	EControllerStatus eStatus;

	//Check to see if the Manager passed in was valid, then check if ID was valid
	if(!_pInputManager)
	{
		eStatus = CONTROLLER_STATUS_INVALID_MANAGER;
	}
	else if(_uiController < XINPUT_MAX_CONTROLLERS)
	{
		//Store which controller we are checking
		m_uiController = _uiController;
		m_pInputManager = _pInputManager;

		//Retrieve the state
		RetrieveState();

		//Check if the controller is connected
		if(m_bIsConnected) eStatus = CONTROLLER_STATUS_CONNECTED;
		else eStatus = CONTROLLER_STATUS_DISCONNECTED;
	}
	else
	{
		//Invalid Controller
		eStatus = CONTROLLER_STATUS_INVALID_ID;
	}

	//Return true if the controller is connected
	return (eStatus);
}

void
CXInputController::Process()
{
	RetrieveState();
}

unsigned int
CXInputController::GetID() const
{
	return (m_uiController);
}

unsigned short
CXInputController::GetButtonStates() const
{
	return (m_usButtonStates);
}

float2
CXInputController::GetLeftThumbPosition() const
{
	//In this function I am making it so that the thumbs start from 0.0f instead of their deadzones.
	float2 vecLeftThumb;
	ZeroMemory(&vecLeftThumb, sizeof(float2));

	//X Axis
	if(m_fLeftThumbStickX > 0.0f) vecLeftThumb.x = ((m_fLeftThumbStickX - m_fLeftThumbStickDeadZone)) + ((m_fLeftThumbStickX * m_fLeftThumbStickDeadZone));
	else if(m_fLeftThumbStickX < 0.0f) vecLeftThumb.x = ((m_fLeftThumbStickX + m_fLeftThumbStickDeadZone)) + ((m_fLeftThumbStickX * m_fLeftThumbStickDeadZone));

	//Y Axis
	if(m_fLeftThumbStickY > 0.0f) vecLeftThumb.y = ((m_fLeftThumbStickY - m_fLeftThumbStickDeadZone)) + ((m_fLeftThumbStickY * m_fLeftThumbStickDeadZone));
	else if(m_fLeftThumbStickY < 0.0f) vecLeftThumb.y = ((m_fLeftThumbStickY + m_fLeftThumbStickDeadZone)) + ((m_fLeftThumbStickY * m_fLeftThumbStickDeadZone));

	return (vecLeftThumb);
}

float2
CXInputController::GetRightThumbPosition() const
{
	//In this function I am making it so that the thumbs start from 0.0f instead of their deadzones.
	float2 vecRightThumb;
	ZeroMemory(&vecRightThumb, sizeof(float2));

	//X Axis
	if(m_fRightThumbStickX > 0.0f)
	{
		float fRightThumbX = ((m_fRightThumbStickX - m_fRightThumbStickDeadZone)) + ((m_fRightThumbStickX * m_fRightThumbStickDeadZone));

		if(m_eAimSetting == CONTROLLER_AIM_CUBIC) vecRightThumb.x = (powf(fRightThumbX, 3.0f) * m_fAimSensitivity);
		else if(m_eAimSetting == CONTROLLER_AIM_LINEAR) vecRightThumb.x = (fRightThumbX * m_fAimSensitivity);
	}
	else if(m_fRightThumbStickX < 0.0f)
	{
		float fRightThumbX = ((m_fRightThumbStickX + m_fRightThumbStickDeadZone)) + ((m_fRightThumbStickX * m_fRightThumbStickDeadZone));

		if(m_eAimSetting == CONTROLLER_AIM_CUBIC) vecRightThumb.x = (powf(fRightThumbX, 3.0f) * m_fAimSensitivity);
		else if(m_eAimSetting == CONTROLLER_AIM_LINEAR) vecRightThumb.x = (fRightThumbX * m_fAimSensitivity);
	}
	else
	{
		//Not moving
	}

	//Y Axis
	if(m_fRightThumbStickY > 0.0f)
	{
		float fRightThumbY = ((m_fRightThumbStickY - m_fRightThumbStickDeadZone)) + ((m_fRightThumbStickY * m_fRightThumbStickDeadZone));

		if(m_eAimSetting == CONTROLLER_AIM_CUBIC) vecRightThumb.y = (powf(fRightThumbY, 3.0f) * m_fAimSensitivity);
		else if(m_eAimSetting == CONTROLLER_AIM_LINEAR) vecRightThumb.y = (fRightThumbY * m_fAimSensitivity);
	}
	else if(m_fRightThumbStickY < 0.0f)
	{
		float fRightThumbY = ((m_fRightThumbStickY + m_fRightThumbStickDeadZone)) + ((m_fRightThumbStickY * m_fRightThumbStickDeadZone));

		if(m_eAimSetting == CONTROLLER_AIM_CUBIC) vecRightThumb.y = (powf(fRightThumbY, 3.0f) * m_fAimSensitivity);
		else if(m_eAimSetting == CONTROLLER_AIM_LINEAR) vecRightThumb.y = (fRightThumbY * m_fAimSensitivity);
	}
	else
	{
		//Not moving
	}

	//Should we invert the look axis?
	if(m_bIsLookYInverted) vecRightThumb.y = -vecRightThumb.y;

	return (vecRightThumb);
}

float
CXInputController::GetLeftTriggerPosition() const
{
	return (m_fLeftTrigger);
}

float
CXInputController::GetRightTriggerPosition() const
{
	return (m_fRightTrigger);
}

void
CXInputController::SetLookYInverted(bool _bInvert)
{
	m_bIsLookYInverted = _bInvert;
}

void
CXInputController::SetAimSensitivity(float _fSensitivity)
{
	m_fAimSensitivity = _fSensitivity;
}

float
CXInputController::GetAimSensitivity() const
{
	return (m_fAimSensitivity);
}

void
CXInputController::SetAimSetting(EControllerAimSettings _eSetting)
{
	m_eAimSetting = _eSetting;
}

EControllerAimSettings
CXInputController::GetAimSetting() const
{
	return (m_eAimSetting);
}

void
CXInputController::SetLeftThumbStickDeadZone(float _fDeadZone)
{
	m_fLeftThumbStickDeadZone = _fDeadZone;

	//Left Thumb Stick value constraining
	ClampValue(m_fLeftThumbStickDeadZone, 0.0f, 1.0f);
}

void
CXInputController::SetRightThumbStickDeadZone(float _fDeadZone)
{
	m_fRightThumbStickDeadZone = _fDeadZone;

	//Right Thumb Stick value constraining
	ClampValue(m_fRightThumbStickDeadZone, 0.0f, 1.0f);
}

float
CXInputController::GetLeftThumbStickDeadZone() const
{
	return (m_fLeftThumbStickDeadZone);
}

float
CXInputController::GetRightThumbStickDeadZone() const
{
	return (m_fRightThumbStickDeadZone);
}

void
CXInputController::SetRumble(float _fLeft, float _fRight)
{
	XINPUT_VIBRATION tVibration;
	ZeroMemory(&tVibration, sizeof(XINPUT_VIBRATION));

	//Store the vibration variables
	m_fLeftRumble = _fLeft;
	m_fRightRumble = _fRight;

	ClampValue(m_fLeftRumble, 0.0f, 1.0f);
	ClampValue(m_fRightRumble, 0.0f, 1.0f);

	//Set the vibration speed, input of 0.0f ~ 1.0f, and convert it to the values needed
	const float fMaxUShort = static_cast<float>(MAXSHORT * 2);
	tVibration.wLeftMotorSpeed = static_cast<unsigned short>(m_fLeftRumble * fMaxUShort);
	tVibration.wRightMotorSpeed = static_cast<unsigned short>(m_fRightRumble * fMaxUShort);

	//Apply the vibration
	XInputSetState(m_uiController, &tVibration);
}

float
CXInputController::GetRumbleLeft() const
{
	return (m_fLeftRumble);
}

float
CXInputController::GetRumbleRight() const
{
	return (m_fRightRumble);
}

EControllerBattery
CXInputController::GetBatteryLevel() const
{
	return (m_eBatteryLevel);
}

bool
CXInputController::IsConnected() const
{
	return (m_bIsConnected);
}

bool
CXInputController::IsWireless() const
{
	return (m_bIsWireless);
}

bool
CXInputController::IsLookYInverted() const
{
	return (m_bIsLookYInverted);
}

bool
CXInputController::IsPressed(unsigned short _usButtons) const
{
	//Cleaner way of checking for multiple key presses
	int iButtonsPressed = 0;
	unsigned short pusButtons[CONTROLLER_MAX_BUTTONS] = {CONTROLLER_DPAD_UP,
															CONTROLLER_DPAD_DOWN,
															CONTROLLER_DPAD_LEFT,
															CONTROLLER_DPAD_RIGHT,
															CONTROLLER_START,
															CONTROLLER_BACK,
															CONTROLLER_LEFT_THUMB,
															CONTROLLER_RIGHT_THUMB,
															CONTROLLER_LEFT_SHOULDER,
															CONTROLLER_RIGHT_SHOULDER,
															CONTROLLER_A,
															CONTROLLER_B,
															CONTROLLER_X,
															CONTROLLER_Y};

	for(int i = 0; i < CONTROLLER_MAX_BUTTONS; ++i)
	{
		//Check if the button has been passed in and check if it is pressed
		if((pusButtons[i] & _usButtons) && (pusButtons[i] & m_usButtonStates))
		{
			//Button pressed
			++iButtonsPressed;
		}
		else if(pusButtons[i] & _usButtons)
		{
			//Button not pressed, break out
			iButtonsPressed = 0;
			break;
		}
		else
		{
			//Not the button we are looking for
		}
	}

	return (iButtonsPressed != 0);
}

void
CXInputController::RetrieveState()
{
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));

	//Attempt to get the state
	DWORD dwResult = XInputGetState(m_uiController, &state);

	//Check if we succeeded
	if(dwResult == ERROR_SUCCESS)
	{
		//Controller is connected
		m_bIsConnected = true;

		//Read all of the data
		ReadBatteryLevel();
		ReadTriggers(state);
		ReadAnalogueSticks(state);
		ReadButtons(state);
	}
	else
	{
		//Controller is no longer connected
		m_bIsConnected = false;

		//Clear all values to prevent issues
		m_fLeftRumble = 0.0f;
		m_fRightRumble = 0.0f;
		m_fLeftTrigger = 0.0f;
		m_fRightTrigger = 0.0f;
		m_fLeftThumbStickX = 0.0f;
		m_fLeftThumbStickY = 0.0f;
		m_fRightThumbStickX = 0.0f;
		m_fRightThumbStickY = 0.0f;
		m_usButtonStates = 0;
	}
}

void
CXInputController::ReadBatteryLevel()
{
	XINPUT_BATTERY_INFORMATION batteryInfo;
	ZeroMemory(&batteryInfo, sizeof(XINPUT_BATTERY_INFORMATION));

	DWORD dwResult = XInputGetBatteryInformation(m_uiController, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo);

	//Check if we succeeded
	if(dwResult == ERROR_SUCCESS)
	{
		m_bIsWireless = (batteryInfo.BatteryType == BATTERY_TYPE_NIMH || batteryInfo.BatteryType == BATTERY_TYPE_ALKALINE);
		m_eBatteryLevel = static_cast<EControllerBattery>(batteryInfo.BatteryLevel);
	}
	else
	{
		//Unknown error, assume its wired
		m_bIsWireless = false;
		m_eBatteryLevel = CONTROLLER_BATTERY_FULL;
	}
}

void
CXInputController::ReadTriggers(XINPUT_STATE& _rState)
{
	//Left and right trigger, float value from 0.0f to 1.0f
	const float fMaxUChar = static_cast<float>(MAXCHAR * 2);
	m_fLeftTrigger = static_cast<float>(_rState.Gamepad.bLeftTrigger) / fMaxUChar;
	m_fRightTrigger = static_cast<float>(_rState.Gamepad.bRightTrigger) / fMaxUChar;

	if(m_fLeftTrigger > 0.0f || m_fRightTrigger > 0.0f)
	{
		if(m_pInputManager)
		{
			if(m_uiController >= 0 && m_uiController <= 3)
			{
				//Set active controller
				m_pInputManager->SetLastActiveDevice(static_cast<EInputDevices>(m_uiController));
			}
			else
			{
				//Invalid ID
			}
		}
		else
		{
			//Invalid Manager
		}
	}
	else
	{
		//Not Pressed
	}
}

void
CXInputController::ReadAnalogueSticks(XINPUT_STATE& _rState)
{
	//Left and right thumbsticks, float values between 0.0f and 1.0f
	const float fMaxShort = static_cast<float>(MAXSHORT);
	m_fLeftThumbStickX = static_cast<float>(_rState.Gamepad.sThumbLX) / fMaxShort;
	m_fLeftThumbStickY = static_cast<float>(_rState.Gamepad.sThumbLY) / fMaxShort;
	m_fRightThumbStickX = static_cast<float>(_rState.Gamepad.sThumbRX) / fMaxShort;
	m_fRightThumbStickY = static_cast<float>(_rState.Gamepad.sThumbRY) / fMaxShort;

	//Thumbstick Deadzone Check
	if((m_fLeftThumbStickX < m_fLeftThumbStickDeadZone) && (m_fLeftThumbStickX > -m_fLeftThumbStickDeadZone)) m_fLeftThumbStickX = 0.0f;
	if((m_fLeftThumbStickY < m_fLeftThumbStickDeadZone) && (m_fLeftThumbStickY > -m_fLeftThumbStickDeadZone)) m_fLeftThumbStickY = 0.0f;
	if((m_fRightThumbStickX < m_fRightThumbStickDeadZone) && (m_fRightThumbStickX > -m_fRightThumbStickDeadZone)) m_fRightThumbStickX = 0.0f;
	if((m_fRightThumbStickY < m_fRightThumbStickDeadZone) && (m_fRightThumbStickY > -m_fRightThumbStickDeadZone)) m_fRightThumbStickY = 0.0f;

	//Controller is being used, update last active
	if(m_fLeftThumbStickX > 0.0f || m_fLeftThumbStickX < 0.0f ||
	   m_fLeftThumbStickY > 0.0f || m_fLeftThumbStickY < 0.0f ||
	   m_fRightThumbStickX > 0.0f || m_fRightThumbStickX < 0.0f ||
	   m_fRightThumbStickY > 0.0f || m_fRightThumbStickY < 0.0f)
	{
		if(m_pInputManager)
		{
			if(m_uiController >= 0 && m_uiController <= 3)
			{
				//Set active controller
				m_pInputManager->SetLastActiveDevice(static_cast<EInputDevices>(m_uiController));
			}
			else
			{
				//Invalid ID
			}
		}
		else
		{
			//Invalid Manager
		}
	}
	else
	{
		//Not Pressed
	}
}

void
CXInputController::ReadButtons(XINPUT_STATE& _rState)
{
	//Get the buttons being pressed, bitwise varaible
	m_usButtonStates = _rState.Gamepad.wButtons;

	if(m_usButtonStates && m_pInputManager)
	{
		if(m_uiController >= 0 && m_uiController <= 3)
		{
			//Set active controller
			m_pInputManager->SetLastActiveDevice(static_cast<EInputDevices>(m_uiController));
		}
		else
		{
			//Invalid ID
		}
	}
	else
	{
		//Not Pressed or Invalid Manager
	}
}