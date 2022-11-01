#pragma once
#ifndef __CLOCK_H__
#define __CLOCK_H__

//Prototypes
class CClock
{
	//Member Functions
public:
	CClock();
	~CClock();

	bool Initialize();

	float GetGameTime() const;
	float GetDeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

	//Member Variables
protected:
	double m_dSecondsPerCount;
	double m_dDeltaTime;

	__int64 m_uiBaseTime;
	__int64 m_uiPausedTime;
	__int64 m_uiStopTime;
	__int64 m_uiPreviousTime;
	__int64 m_uiCurrentTime;

	bool m_bStopped;
};

#endif //__CLOCK_H__