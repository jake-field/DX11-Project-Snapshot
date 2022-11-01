#pragma once
#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

//Library Includes
#include <mutex>
#include <list>

//Prototypes
class ILogTarget;
class CLogManager
{
	//Member Functions
protected:
	CLogManager();
	~CLogManager();

public:
	static CLogManager& GetInstance();
	static void DestroyInstance();

	bool Initialize(bool _bAutoAddTarget = true, bool _bAllowDebug = false);

	void AddTarget(ILogTarget* _pTarget);
	ILogTarget* AddTarget(const char* _pcstrFile);
	ILogTarget* FindTarget(const char* _pcstrID) const;
	void RemoveTarget(ILogTarget* _pTarget);

	void WriteToID(const char* _pcID, const char* _pcstr, const char* _pcprefix = nullptr);
	void WriteToTarget(ILogTarget* _pTarget, const char* _pcstr, const char* _pcprefix = nullptr);
	void WriteAll(const char* _pcstr, const char* _pcprefix = nullptr);
	void WriteDebug(const char* _pcstr, const char* _pcprefix = nullptr);

	//Member Variables
protected:
	static CLogManager* sm_pSelf;

	std::list<ILogTarget*> m_pLogTargets;

	ILogTarget* m_pDebugLog;

	bool m_bAutoAddTarget;

	std::mutex m_mutexWriteLock;

};

#endif //__LOG_MANAGER_H__
