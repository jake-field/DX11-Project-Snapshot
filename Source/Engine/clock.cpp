//Library Includes
#include <windows.h>

//This Include
#include "clock.h"

//Implementation
CClock::CClock()
	: m_dSecondsPerCount(0.0)
	, m_dDeltaTime(0.0)
	, m_uiBaseTime(0)
	, m_uiPausedTime(0)
	, m_uiStopTime(0)
	, m_uiPreviousTime(0)
	, m_uiCurrentTime(0)
	, m_bStopped(false)
{
	__int64 uiCountsPerSecond;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&uiCountsPerSecond));

	m_dSecondsPerCount = 1.0 / static_cast<double>(uiCountsPerSecond);
}

CClock::~CClock()
{
	//Destructor
}

bool
CClock::Initialize()
{
	Reset();
	return (true);
}

float
CClock::GetGameTime() const
{
	double dGameTime = 0.0;

	if(m_bStopped)
	{
		dGameTime = static_cast<double>(m_uiStopTime - m_uiBaseTime) * m_dSecondsPerCount;
	}
	else
	{
		dGameTime = static_cast<double>(m_uiCurrentTime - m_uiPausedTime) * m_dSecondsPerCount;
	}

	return (static_cast<float>(dGameTime));
}

float
CClock::GetDeltaTime() const
{
	return (static_cast<float>(m_dDeltaTime));
}

void
CClock::Reset()
{
	__int64 uiCurrentTime = 0;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&uiCurrentTime));

	m_uiBaseTime = uiCurrentTime;
	m_uiPreviousTime = uiCurrentTime;
	m_uiStopTime = 0;
	m_bStopped = false;
}

void
CClock::Start()
{
	if(m_bStopped)
	{
		__int64 uiStartTime = 0;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&uiStartTime));

		m_uiPausedTime += (uiStartTime - m_uiStopTime);
		m_uiPreviousTime = uiStartTime;
		m_uiStopTime = 0;
		m_bStopped = false;
	}
	else
	{
		//Already Running
	}
}

void
CClock::Stop()
{
	if(!m_bStopped)
	{
		__int64 uiCurrentTime = 0;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&uiCurrentTime));

		m_uiStopTime = uiCurrentTime;
		m_bStopped = true;
	}
	else
	{
		//Already Stopped
	}
}

void
CClock::Tick()
{
	if(!m_bStopped)
	{
		__int64 uiCurrentTime = 0;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&uiCurrentTime));

		m_uiCurrentTime = uiCurrentTime;
		m_dDeltaTime = static_cast<double>(m_uiCurrentTime - m_uiPreviousTime) * m_dSecondsPerCount;
		m_uiPreviousTime = m_uiCurrentTime;

		if(m_dDeltaTime < 0.0)
		{
			m_dDeltaTime = 0.0;
		}
		else
		{
			//Delta tick is fine
		}
	}
	else
	{
		//Can't process tick, stopped.
		m_dDeltaTime = 0.0;
	}
}