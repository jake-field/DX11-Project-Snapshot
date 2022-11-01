#pragma once
#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__

//Library Includes
#include <fstream>

//Local Includes
#include "ilogtarget.h"

//Prototypes
class CLogFile: public ILogTarget
{
	//Member Functions
public:
	CLogFile();
	virtual ~CLogFile();

	bool Initialize(const char* _pcstrFile);
	void Write(const char* _pcstr);
	void WriteLine(const char* _pcstr);

	const char* GetID() const;

	//Member Variables
protected:
	std::ofstream m_logFile;
	char* m_pcstrFile;

};

#endif //__LOG_FILE_H__