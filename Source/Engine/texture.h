#pragma once
#ifndef __TEXTURE_H__
#define __TEXTURE_H__

//Local Includes
#include "renderer.h"
#include "asset.h"

//Data Types
struct TImageData
{
	//Member Variables
	unsigned int	uiWidth;
	unsigned int	uiHeight;
	DXGI_FORMAT		eFormat;
	unsigned int	uiRowPitch; //Image row pitch (how many bytes across before starting a new row)
	unsigned int	uiSlicePitch; //Size in bytes of image as a whole
	uint8_t* pPixelData; //pointer to the Pixel data of specific image (may be located within larger pool of images)

	//Member Functions
	TImageData()
	{
		ZeroMemory(this, sizeof(TImageData));
	}

	void Release()
	{
		if(pPixelData) delete pPixelData;
		ZeroMemory(this, sizeof(TImageData));
	}
};

//Prototype
class CTexture: public IAsset
{
	//Member Functions
public:
	//Returns null if asset not ready
	ID3D11ShaderResourceView* GetSRV() const;

	static EAssetType GetAssetType();

protected:
	CTexture();
	CTexture(const CTexture& _rhs) = default;
	virtual ~CTexture();

	//TODO: Support texturearray
	virtual bool Load(const char* _kpcFilename);
	//virtual bool Load(void* _pData, size_t _size);
	virtual void Release();

private:
	void CreateTextureArray(TImageData* _lptImages, int _iImageCount);
	virtual TImageData LoadImageFromFile(const char* _strFilename);

	//Member Variables
protected:
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pSRView;
	bool m_bIsTextureArray;

	friend CAssetManager;
};

#endif //__TEXTURE_H__
