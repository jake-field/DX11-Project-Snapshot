//Local Includes
#include "logmanager.h"

//This Include
#include "logfile.h"

//Implementation
CLogFile::CLogFile()
	: m_pcstrFile(nullptr)
{
	//Constructor
}

CLogFile::~CLogFile()
{
	//Destructor
	if(m_logFile.is_open()) m_logFile.close();

	if(m_pcstrFile)
	{
		delete[] m_pcstrFile;
		m_pcstrFile = nullptr;
	}
}

bool
CLogFile::Initialize(const char* _pcstrFile)
{
	m_logFile.open(_pcstrFile, std::ios::out | std::ios::trunc);

	if(m_logFile.is_open())
	{
		size_t uiLen = strlen(_pcstrFile) + 1;
		m_pcstrFile = new char[uiLen];
		strcpy_s(m_pcstrFile, uiLen, _pcstrFile);

		CLogManager::GetInstance().WriteDebug("Log file initiation succeeded.\n", _pcstrFile);
	}
	else
	{
		CLogManager::GetInstance().WriteDebug("Log file initiation failed.\n", _pcstrFile);
	}

	return(m_logFile.is_open());
}

void
CLogFile::Write(const char* _pcstr)
{
	if(m_logFile.is_open())
	{
		m_logFile << _pcstr;
		m_logFile.flush(); //Ensure we write in-case of a crash
	}
}

void
CLogFile::WriteLine(const char* _pcstr)
{
	if(m_logFile.is_open())
	{
		m_logFile << _pcstr << std::endl;
		m_logFile.flush(); //Ensure we write in-case of a crash
	}
}

const char* CLogFile::GetID() const
{
	return(m_pcstrFile);
}