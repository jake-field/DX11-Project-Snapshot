#pragma once
#ifndef __ASSET_H__
#define __ASSET_H__

//Library Includes
#include <atomic>
#include <string>

//Macros
#define AssetLoaded(x) (x && x->GetAssetState() == EAssetState::Loaded)

//Types
enum EAssetType
{
	ASSET_TEXTURE,
	ASSET_MODEL,
	//ASSET_SHADER, //TODO: consider implementing this, especially for compiling
	//ASSET_XMLDATA, //TODO: Maybe just file data in general?
	//ASSET_AUDIO,

	EAssetType_MAX,
	EAssetType_ERROR = -1
};

enum class EAssetState
{
	Error,
	Unloaded,
	Queued,
	Loading,
	Loaded
};

//Prototype
class CAssetManager;
class IAsset
{
	//Member Functions
public:
	virtual EAssetState GetAssetState() { return(tm_eAssetState); };
	virtual const char* GetName() { return(m_strAssetName.c_str()); };

protected:
	IAsset()
		: tm_eAssetState(EAssetState::Unloaded)
		, m_strAssetName("")
	{
		//Constructor
	}

	IAsset(const IAsset& _rhs) = default;
	virtual ~IAsset() = default;

	virtual void Release() = 0;
	virtual bool Load(const char* _kpcFilename) = 0;
	//virtual bool Load(void* _pData, size_t _size) = 0;
	//Create()

	//Member Variables
protected:
	std::atomic<EAssetState> tm_eAssetState;
	std::string m_strAssetName;

	friend CAssetManager;

};

#endif //__ASSET_H__
