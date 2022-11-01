//Local Includes
#include "inputmanager.h"

//This Include
#include "freecamera.h"

//Implementation

CFreeCamera::CFreeCamera()
	: m_fMoveSpeed(1.0f)
	, m_fRotSpeed(1.0f)
	, m_bInverted(false)
	, m_vec3LastMousePos(0.0f, 0.0f, 0.0f)
{
	//Constructor
}

CFreeCamera::~CFreeCamera()
{
	//Destructor
}

void
CFreeCamera::Process(float _fDeltaTick)
{
	//TODO: Replace this all with stuff from the Action class, could probably actually get rid of the process function here
	//		if we use ActionEvents.

	//TODO: Change these to axis sensitivities/move speeds etc.
	float fMoveSpeed = m_fMoveSpeed * _fDeltaTick;
	float fRotSpeed = m_fRotSpeed * _fDeltaTick;

	CInputManager& rInput = CInputManager::GetInstance();

	//Shift camera with WASD, CTRL and SPACE
	//This function will force a matrix update
	ShiftPosition(rInput.IsPressed('W')			?  fMoveSpeed : rInput.IsPressed('S')			? -fMoveSpeed : 0.0f,
				  rInput.IsPressed(VK_SPACE)	?  fMoveSpeed : rInput.IsPressed(VK_CONTROL)	? -fMoveSpeed : 0.0f,
				  rInput.IsPressed('A')			? -fMoveSpeed : rInput.IsPressed('D')			?  fMoveSpeed : 0.0f);

	//Mouse rotation for lookat
	float3 vec3MousePos = rInput.GetMousePosition();
	float3 vec3MouseDelta = (vec3MousePos - m_vec3LastMousePos);
	m_vec3LastMousePos = vec3MousePos;

	//Rotate based on mouse delta
	m_vec3Rotation += float3(vec3MouseDelta.y, vec3MouseDelta.x, 0.0f) * (fRotSpeed * rInput.IsPressed(MOUSEBUTTON_LEFT));

	//Process camera last
	__super::Process(_fDeltaTick);
}

void
CFreeCamera::SetMoveSpeed(float _fSpeed)
{
	m_fMoveSpeed = _fSpeed;
}

float
CFreeCamera::GetMoveSpeed() const
{
	return(m_fMoveSpeed);
}

void
CFreeCamera::SetRotSpeed(float _fSpeed)
{
	m_fRotSpeed = _fSpeed;
}

float
CFreeCamera::GetRotSpeed() const
{
	return(m_fRotSpeed);
}

void
CFreeCamera::SetInverted(bool _bInverted)
{
	m_bInverted = _bInverted;
}

bool
CFreeCamera::GetInverted() const
{
	return(m_bInverted);
}