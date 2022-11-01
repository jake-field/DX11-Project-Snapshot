#pragma once
#ifndef __EVENT_EMITTER_H__
#define __EVENT_EMITTER_H__

//Library Includes
#include <vector>

//Prototypes
template <typename EventType>
class CEventEmitter
{
	//Member Functions
public:
	CEventEmitter();
	virtual ~CEventEmitter();

	//Example: Emit([&](IInputEventListener& _rListener) { _rListener.OnKeyPressed(k); });
	template <typename FunctionType>
	void Emit(FunctionType _func);

	virtual void Subscribe(EventType* _pEventListener);
	virtual void Unsubscribe(EventType* _pEventListener);

	//Member Variables
protected:
	std::vector<EventType*> m_pEventListeners;

};

//Implementation
template <typename EventType>
CEventEmitter<EventType>::CEventEmitter()
{
	//Constructor
}

template <typename EventType>
CEventEmitter<EventType>::~CEventEmitter()
{
	//Destructor
	m_pEventListeners.clear(); //We have zero ownership over these
}

template <typename EventType>
template <typename FunctionType>
void CEventEmitter<EventType>::Emit(FunctionType _func)
{
	for(auto pListener : m_pEventListeners) _func(*pListener);
}

template <typename EventType>
void CEventEmitter<EventType>::Subscribe(EventType* _pEventListener)
{
	m_pEventListeners.push_back(_pEventListener);
}

template <typename EventType>
void CEventEmitter<EventType>::Unsubscribe(EventType* _pEventListener)
{
	for(auto i = 0; i < m_pEventListeners.size(); ++i)
	{
		if(m_pEventListeners[i] != _pEventListener) continue;
		m_pEventListeners.erase(m_pEventListeners.begin() + i);
		break;
	}
}

#endif //__EVENT_EMITTER_H__