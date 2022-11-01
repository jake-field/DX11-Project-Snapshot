//Library Includes
#include <chrono> //for thread sleep
#include <objbase.h> //coinit for threading

//Local Includes
#include "logmanager.h"

//This Include
#include "assetmanager.hpp"

//Static Variables
CAssetManager* CAssetManager::sm_pThis = nullptr;

//Implementation
CAssetManager::CAssetManager()
	: m_pRenderer(nullptr)
	, m_bAsyncLoad(false)
	, m_bRunWorkers(false)
{
	//Constructor
}

CAssetManager::~CAssetManager()
{
	//Destructor

	//Close threads to ensure they don't mess up data if something is broken
	m_bRunWorkers = false;
	for (unsigned int i = 0; i < m_vecWorkerThreads.size(); ++i) m_vecWorkerThreads[i].join();

	//Release all assets
	for (unsigned int uiAssetType = 0; uiAssetType < EAssetType_MAX; ++uiAssetType)
	{
		//Release assets in each list
		for (unsigned int uiAssetID = 0; uiAssetID < m_vecStoredAssets[uiAssetType].size(); ++uiAssetID)
		{
			IAsset*& pAsset = m_vecStoredAssets[uiAssetType][uiAssetID];

			pAsset->Release();
			delete pAsset;
			pAsset = nullptr;
		}

		//Clear asset list
		m_vecStoredAssets[uiAssetType].clear();
	}

	//A good hint that we're not loaded is if this is null
	m_pRenderer = nullptr;
}

CAssetManager&
CAssetManager::GetInstance()
{
	//Singleton constructor
	if (!sm_pThis) sm_pThis = new CAssetManager();
	return(*sm_pThis);
}

void
CAssetManager::DestroyInstance()
{
	//Singleton destructor
	if (sm_pThis)
	{
		delete sm_pThis;
		sm_pThis = nullptr;
	}
}

bool
CAssetManager::Initialize(CRenderer* _pRenderer, int _iMaxConcurrentLoading)
{
	m_pRenderer = _pRenderer;
	m_bAsyncLoad = (_iMaxConcurrentLoading > 0);

	//Queue up worker thread for loading assets if m_bAsyncLoad == true
	if (m_bAsyncLoad)
	{
		m_bRunWorkers = true;

		//Create requested number of threads
		for (int i = 0; i < _iMaxConcurrentLoading; ++i)
		{
			m_vecWorkerThreads.push_back(std::thread(CAssetManager::WorkerThread));
		}
	}

	return(m_pRenderer != nullptr);
}

int
CAssetManager::GetQueueLength()
{
	int iQueueLen = 0;
	m_queueMutex.lock();
	iQueueLen = (int)m_queueAssets.size();
	m_queueMutex.unlock();
	return(iQueueLen);
}

CRenderer*
CAssetManager::GetRenderer() const
{
	return(m_pRenderer);
}

void
CAssetManager::WorkerThread()
{
	//TODO: Ability to sleep or end thread when loading isn't required
	// Maybe start a thread each time load is called if there are free thread slots
	// Once thread finishes loading and nothing is queued after 5 seconds, terminate thread
	// This will prevent the need for thread tracking outside of a semaphore for active count against max slots
	std::string debug = "";

	//Dummy
	CoInitialize(nullptr);

	//While the asset manager technically exists and workers should be running
	//TODO: Consider making sm_pThis atomic
	while (sm_pThis != nullptr && sm_pThis->m_bRunWorkers)
	{
		//Try acquire next asset to load, avoid blocking in case of shutdown
		if (sm_pThis->m_queueMutex.try_lock())
		{
			//If the queue is not empty, grab the next asset
			if (!sm_pThis->m_queueAssets.empty())
			{
				IAsset* pAsset = sm_pThis->m_queueAssets.front();
				sm_pThis->m_queueAssets.pop();

				//Unlock to allow other workers to continue
				sm_pThis->m_queueMutex.unlock();

				if (pAsset)
				{
					debug = "Loading asset: " + pAsset->m_strAssetName + "\n";
					CLogManager::GetInstance().WriteDebug(debug.c_str(), "Asset Manager");

					//TODO: This assumes asset name will not change, which can be bad
					pAsset->tm_eAssetState = EAssetState::Loading;
					bool bSuccessful = pAsset->Load(pAsset->m_strAssetName.c_str());

					//Update asset state
					pAsset->tm_eAssetState = bSuccessful ? EAssetState::Loaded : EAssetState::Error;

					std::string debug = bSuccessful ? "Asset Loaded: \"" : "Failed to load asset: \"";
					debug += pAsset->m_strAssetName + "\"\n";
					CLogManager::GetInstance().WriteDebug(debug.c_str(), "Asset Manager");

					//The return doesn't really matter here as Load() should call
					//		to update AssetState if it loaded correctly or failed

					//No point doing this as it will clear the ERROR flag
					//if (!bSuccess) pAsset->Release();
				}
			}
			else
			{
				//Unlock to allow other workers to continue
				sm_pThis->m_queueMutex.unlock();
			}
		}

		//Rest cpu and yield for others
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::this_thread::yield();
	}
}
