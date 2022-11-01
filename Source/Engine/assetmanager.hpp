#pragma once
#ifndef __ASSET_MANAGER_H__
#define __ASSET_MANAGER_H__

//Library Includes
#include <vector>
#include <queue>
#include <thread>
#include <mutex>

//Local Includes
#include "asset.h"

//Protoype
class CRenderer;
class CAssetManager
{
	//Member Functions
public:
	static CAssetManager& GetInstance();
	static void DestroyInstance();

	bool Initialize(CRenderer* _pRenderer, int _iMaxConcurrentLoading = 0);

	//Templated functions for support of any asset
	template<typename TAssetType>
	TAssetType* LoadAsset(const char* _kpcFilename);

	template<typename TAssetType>
	bool UnloadAsset(const char* _kpcName);

	template<typename TAssetType>
	bool UnloadAsset(TAssetType* _pAsset);

	template<typename TAssetType>
	TAssetType* FindAsset(const char* _kpcName);

	int GetQueueLength();
	CRenderer* GetRenderer() const;

private:
	CAssetManager();
	CAssetManager(const CAssetManager& _rhs) = default;
	~CAssetManager();

	static void WorkerThread();

	//Member Variables
protected:
	static CAssetManager* sm_pThis;
	CRenderer* m_pRenderer;
	bool m_bAsyncLoad;

	std::vector<IAsset*> m_vecStoredAssets[EAssetType_MAX];

	std::vector<std::thread> m_vecWorkerThreads;
	std::queue<IAsset*> m_queueAssets;
	std::atomic_bool m_bRunWorkers;
	std::mutex m_queueMutex;

};

//Template Implementation
template<typename TAssetType>
TAssetType* CAssetManager::LoadAsset(const char* _kpcFilename)
{
	TAssetType* pAsset = nullptr;
	EAssetType eType = TAssetType::GetAssetType(); //Assumes type has a static function for returning, fails if not correct

	//Search for already existing
	pAsset = FindAsset<TAssetType>(_kpcFilename);

	//If asset has not been loaded yet
	if (!pAsset)
	{
		//Create new asset of the type, fails to compile if asset is incorrect
		pAsset = new TAssetType;
		pAsset->m_strAssetName = _kpcFilename;

		//Start loading
		if (!m_bAsyncLoad)
		{
			//Attempt to load in the asset
			if (pAsset->Load(_kpcFilename))
			{
				//Asset loaded
				m_vecStoredAssets[eType].push_back(pAsset);
				pAsset->tm_eAssetState = EAssetState::Loaded;
			}
			else
			{
				//Asset failed to load, clean up
				//TODO: Instead of deleting the asset, we could load in the "ERROR" model for rendering
				//		This will allow for friendlier asset errors. Technically the threaded version of this already does half of this.
				pAsset->Release();
				delete pAsset;
				pAsset = nullptr;
			}
		}
		else
		{
			//Load Async
			//Lock the queue for adding an asset to it
			m_queueMutex.lock();

			pAsset->tm_eAssetState = EAssetState::Queued; //Mark asset as queued up
			pAsset->m_strAssetName = _kpcFilename; //File to load async
			m_queueAssets.push(pAsset); //Store to the queue for the workers
			m_vecStoredAssets[eType].push_back(pAsset); //Store to the main asset list

			m_queueMutex.unlock();
		}
	}

	return(pAsset);
}

template<typename TAssetType>
bool CAssetManager::UnloadAsset(const char* _kpcName)
{
	bool bSuccess = false;
	EAssetType eType = TAssetType::GetAssetType(); //Assumes type has a static function for returning, fails if not correct

	//Search for the ID
	for (unsigned int i = 0; i < m_vecStoredAssets[eType].size(); ++i)
	{
		TAssetType* pAsset = (TAssetType*)m_vecStoredAssets[eType][i];

		if (pAsset && !strcmp(pAsset->GetName(), _kpcName))
		{
			//Matched names, that must be us
			pAsset->Release();
			delete pAsset;
			pAsset = nullptr;

			//TODO: Consider changing to std::list to speed up unloading specific assets should this become a perf issue
			m_vecStoredAssets[eType].erase(m_vecStoredAssets[eType].begin() + i);
			bSuccess = true;
			break;
		}
	}

	return(bSuccess);
}

template<typename TAssetType>
bool CAssetManager::UnloadAsset(TAssetType* _pAsset)
{
	bool bSuccess = false;

	//Fail on nullptr before calling Unload()
	bSuccess = _pAsset && UnloadAsset<TAssetType>(_pAsset);

	return(bSuccess);
}

template<typename TAssetType>
TAssetType* CAssetManager::FindAsset(const char* _kpcName)
{
	TAssetType* pAsset = nullptr;
	EAssetType eType = TAssetType::GetAssetType(); //Assumes type has a static function for returning, fails if not correct

	//Search for already existing
	for (unsigned int i = 0; i < m_vecStoredAssets[eType].size(); ++i)
	{
		TAssetType* pTargetAsset = (TAssetType*)m_vecStoredAssets[eType][i];

		if (pTargetAsset && !strcmp(pTargetAsset->GetName(), _kpcName))
		{
			//Matched names, assume asset exists
			pAsset = pTargetAsset;
			break;
		}
	}

	return(pAsset);
}

#endif //__ASSET_MANAGER_H__
