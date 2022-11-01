//Library Includes
#include <mutex>

//Local Includes
#include "ilogtarget.h"
#include "logdebug.h"
#include "logfile.h"

//This Include
#include "logmanager.h"

//Static
CLogManager* CLogManager::sm_pSelf = nullptr;

//Implementation
CLogManager::CLogManager()
	: m_pDebugLog(nullptr)
	, m_bAutoAddTarget(true)
{
	//Constructor
}

CLogManager::~CLogManager()
{
	//Destructor
	for(auto pLogTarget : m_pLogTargets)
	{
		delete pLogTarget;
	}

	m_pLogTargets.clear();

	//Trash the debug log if it was created
	if(m_pDebugLog)
	{
		delete m_pDebugLog;
		m_pDebugLog = nullptr;
	}
}

CLogManager&
CLogManager::GetInstance()
{
	if(!sm_pSelf)
	{
		sm_pSelf = new CLogManager();
	}

	return(*sm_pSelf);
}

void
CLogManager::DestroyInstance()
{
	if(sm_pSelf) delete sm_pSelf;
	sm_pSelf = nullptr;
}

bool
CLogManager::Initialize(bool _bAutoAddTarget, bool _bAllowDebug)
{
	bool bSuccessful = true;
	m_bAutoAddTarget = _bAutoAddTarget;

	if(_bAllowDebug)
	{
		m_pDebugLog = new CLogDebug;
		if(m_pDebugLog) ((CLogDebug*)m_pDebugLog)->Initialize(); //TODO: This is bad, debug.log is failing and we're ignoring it
	}

	return(bSuccessful);
}

void
CLogManager::AddTarget(ILogTarget* _pTarget)
{
	m_pLogTargets.push_back(_pTarget);
}

ILogTarget*
CLogManager::AddTarget(const char* _pcstrFile)
{
	CLogFile* pTarget = new CLogFile;

	if(pTarget != nullptr && pTarget->Initialize(_pcstrFile))
	{
		AddTarget(pTarget);
	}
	else if(pTarget)
	{
		delete pTarget;
		pTarget = nullptr;
	}

	return(pTarget);
}

ILogTarget*
CLogManager::FindTarget(const char* _pcstrID) const
{
	ILogTarget* pRetTarget = nullptr;

	for(ILogTarget* pLogTarget : m_pLogTargets)
	{
		//Case-insensitive comparing ID's
		if(_strcmpi(pLogTarget->GetID(), _pcstrID) == 0)
		{
			pRetTarget = pLogTarget;
			break;
		}
	}

	return(pRetTarget);
}

void
CLogManager::RemoveTarget(ILogTarget* _pTarget)
{
	m_pLogTargets.remove(_pTarget);
}

void
CLogManager::WriteToID(const char* _pcID, const char* _pcstr, const char* _pcprefix)
{
	ILogTarget* pTarget = FindTarget(_pcID);
	if(!pTarget && m_bAutoAddTarget) pTarget = AddTarget(_pcID);
	
	if(pTarget)
	{
		WriteToTarget(pTarget, _pcstr, _pcprefix);
	}
	else if(m_pDebugLog) //Debug check
	{
		char pcMessage[64 + 19];
		strcpy_s(pcMessage, _pcID);
		strcat_s(pcMessage, " is not a valid ID\n");
		WriteDebug(pcMessage, "CLogManager::WriteToID()");
	}
}

void
CLogManager::WriteToTarget(ILogTarget* _pTarget, const char* _pcstr, const char* _pcprefix)
{
	if(_pTarget != nullptr)
	{
		m_mutexWriteLock.lock();

		if(_pcprefix != nullptr)
		{
			_pTarget->Write(_pcprefix);
			_pTarget->Write("> ");
		}

		_pTarget->Write(_pcstr);

		m_mutexWriteLock.unlock();
	}
	else if(m_pDebugLog) //Debug check
	{
		WriteDebug("ILogTarget* _pTarget does not exist\n", "CLogManager::WriteToTarget()");
	}
}

void
CLogManager::WriteAll(const char* _pcstr, const char* _pcprefix)
{
	m_mutexWriteLock.lock();

	for(ILogTarget* pLogTarget : m_pLogTargets)
	{
		if(_pcprefix != nullptr)
		{
			pLogTarget->Write(_pcprefix);
			pLogTarget->Write("> ");
		}

		pLogTarget->Write(_pcstr);
	}

	m_mutexWriteLock.unlock();

	WriteDebug(_pcstr, _pcprefix);
}

void
CLogManager::WriteDebug(const char* _pcstr, const char* _pcprefix)
{
	m_mutexWriteLock.lock();

	if(m_pDebugLog)
	{
		if(_pcprefix != nullptr)
		{
			m_pDebugLog->Write(_pcprefix);
			m_pDebugLog->Write("> ");
		}

		m_pDebugLog->Write(_pcstr);
	}

	m_mutexWriteLock.unlock();
}
