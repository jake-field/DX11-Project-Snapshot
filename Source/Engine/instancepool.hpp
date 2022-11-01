#pragma once
#ifndef __INSTANCEPOOL_H__
#define __INSTANCEPOOL_H__

//Local Includes
#include "ishader.h"
#include "iinstancepool.h"
#include "renderer.h"

//Template assistance, saves changing implementation code by hand
#define CINSTANCEPOOL_INSERT TInstanceType
#define CINSTANCEPOOL_TEMPLATE template<typename CINSTANCEPOOL_INSERT>

//Prototypes
CINSTANCEPOOL_TEMPLATE
class CInstancePool final: public IInstancePool
{
	//Member Functions
public:
	CInstancePool();
	virtual ~CInstancePool();

	//_uiInstanceCount must match length of _ptInstanceData if not nullptr.
	//If _ptInstanceData isn't passed, _uiInstanceCount is the size of the buffer and will need to be filled
	//_bReadable lets the buffer be readable locally. If not readable, trust only AppendInstance works (adds to pool of _uiInstanceCount), not Set/Get
	bool Initialize(CRenderer* _pRenderer, TInstanceType* _ptInstanceData, unsigned int _uiInstanceCount, bool _bReadable = false);

	//Unlocks the buffer for writing
	//If readable, (un)lock() will not be required as GetBuffer will cause a copy if data doesn't match
	//Append will start from end of the buffer if readable and !writediscard. Otherwise Append will start at 0
	//WriteDiscard will cause GetInstanceCount to equal last index written to, even if there is valid data in the remaining buffer
	bool Unlock(bool _bWriteDiscard = false);

	//Locks the buffer so that CMesh can call DrawInstanced() & BindToIA()
	//Calling GetBuffer will lock/close the buffer automatically
	bool Lock(unsigned int _uiTotalCount = -1); //_uiTotalCount is buffer termination, -1 = m_uiInstanceCount or m_uiPrevIndex w/e >=

	//Appends instances provided to the end of the buffer
	bool AppendInstances(TInstanceType* _ptData, unsigned int _uiInstanceCount);

	//Configure/Retrieve the instance(s) when the buffer is 'readable', fails if buffer is not readable
	bool SetInstances(unsigned int _uiStartIndex, TInstanceType* _ptData, unsigned int _uiInstanceCount);
	const TInstanceType* const GetInstance(unsigned int _uiIndex) const;
	
	//Functions used by CMesh for instanced drawing
	unsigned int GetMax() const; //Maximum size of the buffer
	unsigned int GetValid() const; //Number of valid objects in the buffer for drawing
	unsigned int GetStride() const; //Stride for IA stage
	ID3D11Buffer* const GetBuffer(); //Buffer for IA stage, calling this forces CloseBuffer() to be called
	bool IsUpdated() const;

	//Member Variables
protected:
	//Renderer
	CRenderer* m_pRenderer;

	//Readable instance data
	TInstanceType* m_ptInstanceData; //Only valid if _bReadable set at init, otherwise data is directly written to the buffer
	unsigned int m_uiInstanceCount; //Length of m_ptInstanceData. Size is interpretted by template

	//Last location in instance data, this is the [0,n] of usable data in the buffer
	unsigned int m_uiPrevIndex;

	//Buffer variables
	ID3D11Buffer* m_pBuffer;
	D3D11_MAPPED_SUBRESOURCE m_pMappedBuffer;
	bool m_bUpdateBuffer;

};


//------------------------------------------------------------------------
//Implementation
//------------------------------------------------------------------------
CINSTANCEPOOL_TEMPLATE
CInstancePool<CINSTANCEPOOL_INSERT>::CInstancePool()
	: m_pRenderer(nullptr)
	, m_ptInstanceData(nullptr)
	, m_uiInstanceCount(0)
	, m_uiPrevIndex(0)
	, m_pBuffer(nullptr)
	, m_bUpdateBuffer(false)
{
	//Constructor
	ZeroMemory(&m_pMappedBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
}

CINSTANCEPOOL_TEMPLATE
CInstancePool<CINSTANCEPOOL_INSERT>::~CInstancePool()
{
	//Destructor
	m_pRenderer = nullptr;
	m_uiInstanceCount = 0;
	m_uiPrevIndex = 0;
	m_bUpdateBuffer = false;

	ReleaseCOM(m_pBuffer);
	SafeDeleteArray(m_ptInstanceData);
	ZeroMemory(&m_pMappedBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
}

CINSTANCEPOOL_TEMPLATE
bool CInstancePool<CINSTANCEPOOL_INSERT>::Initialize(CRenderer* _pRenderer, TInstanceType* _ptInstanceData, unsigned int _uiInstanceCount, bool _bReadable)
{
	m_pRenderer = _pRenderer;

	//Re-init checks
	if(m_pBuffer || m_ptInstanceData)
	{
		Lock(); //Silent fail, but doesn't hurt to call
		ReleaseCOM(m_pBuffer);
		ZeroMemory(&m_pMappedBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
		SafeDeleteArray(m_ptInstanceData);
		m_uiInstanceCount = 0;
	}

	//If readable, create the instance storage array (m_ptInstanceData)
	if(_bReadable)
	{
		unsigned int uiSize = m_uiInstanceCount * sizeof(TInstanceType);
		m_ptInstanceData = new TInstanceType[_uiInstanceCount];
		m_uiInstanceCount = _uiInstanceCount;
		m_bUpdateBuffer = false;
		m_uiPrevIndex = 0;

		//Copy the data if we were provided with it, otherwise zero the memory
		if(_ptInstanceData) memcpy_s(m_ptInstanceData, uiSize, _ptInstanceData, uiSize);
		else ZeroMemory(m_ptInstanceData, uiSize);
	}

	//Create the instance buffer
	if(m_pRenderer && m_pRenderer->GetDeviceContext())
	{
		//TODO: if _ptInstanceData is null, consider using zero'd out m_ptInstanceData and then delete if !_bReadable
		//		I am unsure if this will cause an error until I test it, if it doesn't crash then we good
		//TODO: Consider D3D11_USAGE_DEFAULT/IMMUTABLE vs. D3D11_USAGE_DYNAMIC in the case of instance data that will never change
		//		This could be useful for pre-configured instance scenes where they use a grid layout for optimized drawing
		m_pRenderer->GetGPUMutex().lock(); //Required for CreateBuffer as we may conflict with multi-threaded parts of code
		m_pBuffer = m_pRenderer->CreateBuffer(D3D11_BIND_VERTEX_BUFFER, _ptInstanceData, _uiInstanceCount * sizeof(TInstanceType), D3D11_USAGE_DYNAMIC);
		m_pRenderer->GetGPUMutex().unlock();
	}

	return(m_pBuffer != nullptr);
}

CINSTANCEPOOL_TEMPLATE
bool CInstancePool<CINSTANCEPOOL_INSERT>::Unlock(bool _bWriteDiscard)
{
	//Pointer check, also skip if buffer is already unlocked
	if(m_pRenderer && m_pRenderer->GetDeviceContext() && m_pBuffer && !m_pMappedBuffer.pData)
	{
		//If _bWriteDiscard, or if we are not readable (buffer opens in WD anyway...), reset the prev index to 0
		if(_bWriteDiscard || !m_ptInstanceData) m_uiPrevIndex = 0; //Reset instance index to 0

		//Ignore HR as we can test against pData
		m_pRenderer->GetDeviceContext()->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_pMappedBuffer);
	}

	//Return true if buffer is unlocked
	return(m_pMappedBuffer.pData != nullptr);
}

CINSTANCEPOOL_TEMPLATE
bool CInstancePool<CINSTANCEPOOL_INSERT>::Lock(unsigned int _uiTotalCount)
{
	//////////////
	// Code here is commented out as technically Append/Set do the following for optimized updating.
	// If we do the following code, we could be doing a memcpy on 5000+ objects when only 4 items are
	// needed for updating. So the following code causes this pool to be stupidly slow for no reason.
	// This code is replaced by the following uncommented if statement.
	//////////////
	//If autofill (-1)
	//if(_uiTotalCount == (unsigned int)(-1))
	//{
	//	//If readable, use full length of buffer, otherwise leave m_uiPrevIndex unchanged (set by AppendInstances)
	//	if(m_ptInstanceData) m_uiPrevIndex = m_uiInstanceCount;
	//}
	//else
	//{
	//	//Specific count given, capped to maximum buffer size
	//	m_uiPrevIndex = _uiTotalCount > m_uiInstanceCount ? m_uiInstanceCount : _uiTotalCount;
	//}

	//Pointer check, also skip if buffer is already locked as it's uneditable
	if(m_pRenderer && m_pRenderer->GetDeviceContext() && m_pBuffer && m_pMappedBuffer.pData)
	{
		//If we're given a specific number, match that number or our max, whichever is lesser
		if(_uiTotalCount != (unsigned int)(-1)) m_uiPrevIndex = _uiTotalCount > m_uiInstanceCount ? m_uiInstanceCount : _uiTotalCount;

		//If we need to update and have readable data
		if(m_bUpdateBuffer && m_ptInstanceData)
		{
			//Copy valid data into buffer. Could probably fill 1:1 but if it's 2048 and we only need to copy 24 items, speed may become a factor
			int iRet = memcpy_s(m_pMappedBuffer.pData, m_uiInstanceCount * sizeof(TInstanceType), m_ptInstanceData, m_uiPrevIndex * sizeof(TInstanceType));
			m_bUpdateBuffer = (iRet != 0); //If true, memcpy_s failed and we need to update again
		}

		//Ignore HR as we can test against pData
		m_pRenderer->GetDeviceContext()->Unmap(m_pBuffer, 0);
		m_pMappedBuffer.pData = nullptr;
	}

	//If m_bUpdateBuffer is true, then we failed to successfully set & lock.
	return(!m_bUpdateBuffer);
}

CINSTANCEPOOL_TEMPLATE
bool CInstancePool<CINSTANCEPOOL_INSERT>::AppendInstances(TInstanceType* _ptData, unsigned int _uiInstanceCount)
{
	return(SetInstances(-1, _ptData, _uiInstanceCount));
}

CINSTANCEPOOL_TEMPLATE
bool CInstancePool<CINSTANCEPOOL_INSERT>::SetInstances(unsigned int _uiStartIndex, TInstanceType* _ptData, unsigned int _uiInstanceCount)
{
	bool bSuccess = false;
	void* pStart = nullptr;
	const size_t kuiSize = sizeof(TInstanceType);
	unsigned int uiOffset = (_uiStartIndex == (unsigned int)(-1)) ? m_uiPrevIndex : _uiStartIndex;

	//If within memory limits
	if((m_uiPrevIndex + _uiInstanceCount) <= m_uiInstanceCount)
	{
		//If readable, copy to local memory only and flag update (done in Lock)
		if(m_ptInstanceData)
		{
			pStart = &m_ptInstanceData[uiOffset];
			m_bUpdateBuffer = true; //Don't bother updating buffer as we do a semi-optimized one on Lock()
		}
		else if(m_pMappedBuffer.pData) //Copy direct to open buffer as not readable
		{
			pStart = &((TInstanceType*)m_pMappedBuffer.pData)[uiOffset];
		}

		//Copy if we have a target. If memcpy fails, set pStart to nullptr which will cause this func to return false
		if(pStart) bSuccess = !memcpy_s(pStart, m_uiInstanceCount * kuiSize, _ptData, _uiInstanceCount * kuiSize);

		//Update previous index if this number is > the old.
		//If we are readable, m_uiPrevIndex will never go backwards which means this code never gets faster unless
		//		we store a boolean and uint for between lock checking of index discrepencies, but that can lead to
		//		undesired results so it's better left the way it is now...
		if(bSuccess && (uiOffset + _uiInstanceCount) > m_uiPrevIndex) m_uiPrevIndex = (uiOffset + _uiInstanceCount);
	}

	//Returns true if we successfully made the copy
	return(bSuccess);
}

CINSTANCEPOOL_TEMPLATE
const TInstanceType* const CInstancePool<CINSTANCEPOOL_INSERT>::GetInstance(unsigned int _uiIndex) const
{
	//Return from the instance array if it exists (if we are readable), otherwise return nullptr
	return(m_ptInstanceData ? m_ptInstanceData[_uiIndex] : nullptr);
}

CINSTANCEPOOL_TEMPLATE
unsigned int CInstancePool<CINSTANCEPOOL_INSERT>::GetMax() const
{
	return(m_uiInstanceCount);
}

CINSTANCEPOOL_TEMPLATE
unsigned int CInstancePool<CINSTANCEPOOL_INSERT>::GetValid() const
{
	return(m_uiPrevIndex);
}

CINSTANCEPOOL_TEMPLATE
unsigned int CInstancePool<CINSTANCEPOOL_INSERT>::GetStride() const
{
	return(sizeof(TInstanceType));
}

CINSTANCEPOOL_TEMPLATE
ID3D11Buffer* const CInstancePool<CINSTANCEPOOL_INSERT>::GetBuffer()
{
	//TODO: Consider the implications of locking here, probably should return nullptr if the buffer is still open for editing
	//		If we change, make to sure re-const the function
	if(m_pMappedBuffer.pData) Lock();
	return(m_pBuffer);
}

CINSTANCEPOOL_TEMPLATE
bool CInstancePool<CINSTANCEPOOL_INSERT>::IsUpdated() const
{
	return(!m_bUpdateBuffer);
}

#endif //__INSTANCEPOOL_H__
