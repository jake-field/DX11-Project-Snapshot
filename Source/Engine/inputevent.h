#pragma once
#ifndef __INPUT_EVENT_H__
#define __INPUT_EVENT_H__

//Prototypes
class CInputManager;
class IInputEventListener
{
	//Member Functions
protected:
	IInputEventListener();
	virtual ~IInputEventListener();

private:
	//Empty funcs destined to be overriden
	virtual void OnKeyPressed(char _key) {};
	virtual void OnKeyReleased(char _key) {};

	//Friend for wide support
	friend CInputManager;

};

#endif //__INPUT_EVENT_H__