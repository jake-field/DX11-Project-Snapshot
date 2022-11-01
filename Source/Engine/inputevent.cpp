//Local Includes
#include "inputmanager.h"

//This Include
#include "inputevent.h"

//Implementation
IInputEventListener::IInputEventListener()
{
	//Throw if the input manager isn't valid, prevents creation of the singleton if it's been destroyed
	if(!CInputManager::IsValid()) throw false;
	CInputManager::GetInstance().Subscribe(this);
};

IInputEventListener::~IInputEventListener()
{
	//Avoid calling GetInstance() if the input manager is already destroyed
	if(CInputManager::IsValid()) CInputManager::GetInstance().Unsubscribe(this);
};