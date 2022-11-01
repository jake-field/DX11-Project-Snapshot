//Local Includes
#include "logfile.h"
#include "consolewindow.h"

//This Include
#include "logdebug.h"

//Implementation
CLogDebug::CLogDebug()
	: m_pLogFile(nullptr)
{
	//Constructor
}

CLogDebug::~CLogDebug()
{
	//Destructor
	CConsoleWindow::DestroyInstance();
	
	if(m_pLogFile)
	{
		delete m_pLogFile;
		m_pLogFile = nullptr;
	}
}

bool
CLogDebug::Initialize(bool _bWriteToFile)
{
	bool bSuccess = CConsoleWindow::GetInstance().Initialize();

	if(_bWriteToFile)
	{
		CLogFile* pLogFile = new CLogFile;

		if(pLogFile)
		{
			if(pLogFile->Initialize("debug.log"))
			{
				m_pLogFile = pLogFile;
			}
			else
			{
				WriteLine("Unable to open debug.log");
				delete pLogFile;
				pLogFile = nullptr;
			}
		}
	}

	//Check if log file was open if requested
	bSuccess = bSuccess && ((_bWriteToFile && m_pLogFile != nullptr) || !_bWriteToFile);

	return(bSuccess);
}

void
CLogDebug::Write(const char* _pcstr)
{
	if(CConsoleWindow::GetConsoleState() != EConsoleState::CONSOLE_INACTIVE) CConsoleWindow::GetInstance().Write(_pcstr);
	if(m_pLogFile != nullptr) m_pLogFile->Write(_pcstr);

	#ifdef _DEBUG
	OutputDebugStringA(_pcstr);
	#endif //_DEBUG
}

void CLogDebug::WriteLine(const char* _pcstr)
{
	Write(_pcstr);
	Write("\n");
}

const char* CLogDebug::GetID() const
{
	const char* pcstr = "DEBUG";

	//Attempt to use log file ID if it exists
	if(m_pLogFile != nullptr) pcstr = m_pLogFile->GetID();

	return(pcstr);
}