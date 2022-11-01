#pragma once
#ifndef __FREE_CAMERA_H__
#define __FREE_CAMERA_H__

//Local Includes
#include "camera.h"
//#include "inputevent.h"

//Prototypes
class CFreeCamera: public CCamera//, public IInputEventListener
{
	//Member Functions
public:
	CFreeCamera();
	virtual ~CFreeCamera();

	virtual void Process(float _fDeltaTick);

	void SetMoveSpeed(float _fSpeed);
	float GetMoveSpeed() const;

	void SetRotSpeed(float _fSpeed);
	float GetRotSpeed() const;

	void SetInverted(bool _bInverted);
	bool GetInverted() const;

protected:
	//TODO: instead of input event listener, use IActionEventListener; OnAction(LXAxis, LYAxis, RightXAxis, RYAxis)?
	//virtual void OnKeyPressed(char _key);
	//virtual void OnKeyReleased(char _key);

	//Member Variables
protected:
	float m_fMoveSpeed;
	float m_fRotSpeed;

	bool m_bInverted;
	//bool m_bRotating; //TODO: remove this when swapping to OnAction with digital inputs. This is just for mouse prev-current
	float3 m_vec3LastMousePos;

};

#endif //__FREE_CAMERA_H__