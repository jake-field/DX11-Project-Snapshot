#pragma once
#ifndef __LOG_DEBUG_H__
#define __LOG_DEBUG_H__

//Local Includes
#include "ilogtarget.h"

//Prototypes
class CLogDebug: public ILogTarget
{
	//Member Functions
public:
	CLogDebug();
	virtual ~CLogDebug();

	bool Initialize(bool _bWriteToFile = true);
	void Write(const char* _pcstr);
	void WriteLine(const char* _pcstr);

	const char* GetID() const;

	//Member Variables
protected:
	ILogTarget* m_pLogFile;

};

#endif //__LOG_DEBUG_H__