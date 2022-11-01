//Local Includes
#include "wichelper.h"
#include "assetmanager.hpp"

//This Include
#include "texture.h"

//Implementation
CTexture::CTexture()
	: m_pTexture(nullptr)
	, m_pSRView(nullptr)
	, m_bIsTextureArray(false)
{
	//Constructor
}

CTexture::~CTexture()
{
	//Destructor
}

ID3D11ShaderResourceView*
CTexture::GetSRV() const
{
	return(m_pSRView);
}

EAssetType
CTexture::GetAssetType()
{
	return(ASSET_TEXTURE);
}

bool
CTexture::Load(const char* _kpcFilename)
{
	CRenderer* pRenderer = CAssetManager::GetInstance().GetRenderer();

	if (pRenderer)
	{
		TImageData pTempImage = LoadImageFromFile(_kpcFilename);

		if (pTempImage.pPixelData != nullptr)
		{
			//TODO: Allow loading of multiple textures
			//Attempt to create dxtexture from image
			CreateTextureArray(&pTempImage, 1);

			//Delete data
			pTempImage.Release();
		}
		else
		{
			//Failed to create the image array
		}
	}

	//Return true if texture was valid
	return(m_pTexture != nullptr);
}

void
CTexture::Release()
{
	if (m_pTexture) m_pTexture->Release();
	if (m_pSRView) m_pSRView->Release();
	m_pTexture = nullptr;
	m_pSRView = nullptr;
}

void
CTexture::CreateTextureArray(TImageData* _lptImages, int _iImageCount)
{
	HRESULT hr = S_OK;
	CRenderer* pRenderer = CAssetManager::GetInstance().GetRenderer();
	m_bIsTextureArray = true;

	//Acquire a lock so we can allocate assets
	pRenderer->GetGPUMutex().lock();

	//Texture description
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = _lptImages[0].uiWidth;
	desc.Height = _lptImages[0].uiHeight;
	desc.MipLevels = 1;
	desc.ArraySize = _iImageCount;
	desc.Format = _lptImages[0].eFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	//Allocate intial data buffer
	D3D11_SUBRESOURCE_DATA* initData = new D3D11_SUBRESOURCE_DATA[desc.ArraySize];

	//For each image provided, prep subresource data.
	for (unsigned int i = 0; i < desc.ArraySize; ++i)
	{
		initData[i].pSysMem = _lptImages[i].pPixelData;
		initData[i].SysMemPitch = _lptImages[i].uiRowPitch;
		initData[i].SysMemSlicePitch = _lptImages[i].uiSlicePitch;
	}

	//Attempt to create the texture
	hr = pRenderer->GetDevice()->CreateTexture2D(&desc, initData, &m_pTexture);

	if (SUCCEEDED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		SRVDesc.Format = desc.Format;

		//Only difference for loading into a texture array
		if (desc.ArraySize > 1)
		{
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			SRVDesc.Texture2DArray.ArraySize = desc.ArraySize;
			SRVDesc.Texture2DArray.MipLevels = 1;
		}
		else
		{
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;
		}

		//Create the shader resource for the texture
		hr = pRenderer->GetDevice()->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pSRView);

		if (FAILED(hr))
		{
			//Failed to create the shader resource
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
	}
	else
	{
		//No texture created
	}

	//Delete temp data
	if (initData)
	{
		delete[] initData;
		initData = nullptr;
	}

	//Release the lock as we are done allocating
	pRenderer->GetGPUMutex().unlock();
}

TImageData
CTexture::LoadImageFromFile(const char* _strFilename)
{
	HRESULT hr = S_OK;
	unsigned int uiBitsPerPixel = 0;
	unsigned int uiMaxSize = 0;
	unsigned int uiImportWidth = 0;
	unsigned int uiImportHeight = 0;
	unsigned int uiOutWidth = 0;
	unsigned int uiOutHeight = 0;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pBitmapFrame = nullptr;
	IWICImagingFactory* pWIC = _GetWIC();
	CRenderer* pRenderer = CAssetManager::GetInstance().GetRenderer();

	//Covert filename to a wchar_t for WIC
	size_t uiLength = 0;
	wchar_t wstrFilename[128];
	mbstowcs_s(&uiLength, wstrFilename, _strFilename, 128);

	//WIC Decoder
	hr = pWIC->CreateDecoderFromFilename(wstrFilename, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
	assert(SUCCEEDED(hr));

	//Get Bitmap frame
	hr = pDecoder->GetFrame(0, &pBitmapFrame);
	assert(SUCCEEDED(hr));

	//Attempt to get the size of the image
	hr = pBitmapFrame->GetSize(&uiImportWidth, &uiImportHeight);
	assert(SUCCEEDED(hr));

	//Maximum size supported
	switch (pRenderer->GetDevice()->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_9_1:
	case D3D_FEATURE_LEVEL_9_2:
		uiMaxSize = 2048;
		break;
	case D3D_FEATURE_LEVEL_9_3:
		uiMaxSize = 4096;
		break;
	case D3D_FEATURE_LEVEL_10_0:
	case D3D_FEATURE_LEVEL_10_1:
		uiMaxSize = 8192;
		break;
	default:
		uiMaxSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		break;
	}

	//Scale down the image to match uiMaxSize
	if (uiImportWidth > uiMaxSize || uiImportHeight > uiMaxSize)
	{
		float ar = static_cast<float>(uiImportHeight) / static_cast<float>(uiImportWidth);
		if (uiImportWidth > uiImportHeight)
		{
			uiOutWidth = uiMaxSize;
			uiOutHeight = static_cast<unsigned int>(static_cast<float>(uiMaxSize) * ar);
		}
		else
		{
			uiOutHeight = uiMaxSize;
			uiOutWidth = static_cast<unsigned int>(static_cast<float>(uiMaxSize) / ar);
		}
	}
	else
	{
		uiOutWidth = uiImportWidth;
		uiOutHeight = uiImportHeight;
	}

	// Determine format
	WICPixelFormatGUID pixelFormat;
	hr = pBitmapFrame->GetPixelFormat(&pixelFormat);
	assert(SUCCEEDED(hr));

	//Copy pixel format into convertGUID
	WICPixelFormatGUID convertGUID;
	memcpy(&convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID));

	//Try determine DXGI format
	DXGI_FORMAT eFormat = _WICToDXGI(pixelFormat);
	if (eFormat == DXGI_FORMAT_UNKNOWN)
	{
		for (unsigned int i = 0; i < _countof(g_WICConvert); ++i)
		{
			if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
			{
				memcpy(&convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID));

				eFormat = _WICToDXGI(g_WICConvert[i].target);
				assert(eFormat != DXGI_FORMAT_UNKNOWN);
				uiBitsPerPixel = (unsigned int)_WICBitsPerPixel(convertGUID);
				break;
			}
		}

		if (eFormat == DXGI_FORMAT_UNKNOWN)
		{
			//Failed to resolve DX formate
		}
	}
	else
	{
		uiBitsPerPixel = (unsigned int)_WICBitsPerPixel(pixelFormat);
	}

	//Successfuly got bits per pixel?
	assert(uiBitsPerPixel);

	// Verify our target format is supported by the current device
	// (handles WDDM 1.0 or WDDM 1.1 device driver cases as well as DirectX 11.0 Runtime without 16bpp format support)
	unsigned int support = 0;
	hr = pRenderer->GetDevice()->CheckFormatSupport(eFormat, &support);
	if (FAILED(hr) || !(support & D3D11_FORMAT_SUPPORT_TEXTURE2D))
	{
		// Fallback to RGBA 32-bit format which is supported by all devices
		memcpy(&convertGUID, &GUID_WICPixelFormat32bppRGBA, sizeof(WICPixelFormatGUID));
		eFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		uiBitsPerPixel = 32;
	}

	// Allocate temporary memory for image
	unsigned int uiRowPitch = (uiOutWidth * uiBitsPerPixel + 7) / 8; //TODO: Consider ComputePitch from DXTex for greater support
	unsigned int uiImageSize = uiRowPitch * uiOutHeight;

	//Create a new image
	TImageData tUsableImage;
	tUsableImage.uiRowPitch = uiRowPitch;
	tUsableImage.uiSlicePitch = uiImageSize;
	tUsableImage.uiWidth = uiOutWidth;
	tUsableImage.uiHeight = uiOutHeight;
	tUsableImage.eFormat = eFormat;
	tUsableImage.pPixelData = new uint8_t[uiImageSize];

	//Check if we need to perform anything on the imported image to make it usable for DX11
	if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0 && uiOutWidth == uiImportWidth && uiOutHeight == uiImportHeight)
	{
		// No format conversion or resize needed
		hr = pBitmapFrame->CopyPixels(0, uiRowPitch, uiImageSize, tUsableImage.pPixelData);
		assert(SUCCEEDED(hr));
	}
	else if (uiOutWidth != uiImportWidth || uiOutHeight != uiImportHeight)
	{
		// Resize
		IWICBitmapScaler* scaler;
		hr = pWIC->CreateBitmapScaler(&scaler);
		assert(SUCCEEDED(hr));

		hr = scaler->Initialize(pBitmapFrame, uiOutWidth, uiOutHeight, WICBitmapInterpolationModeFant);
		assert(SUCCEEDED(hr));

		WICPixelFormatGUID pfScaler;
		hr = scaler->GetPixelFormat(&pfScaler);
		assert(SUCCEEDED(hr));

		if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
		{
			// No format conversion needed
			hr = scaler->CopyPixels(0, uiRowPitch, uiImageSize, tUsableImage.pPixelData);
			assert(SUCCEEDED(hr));
		}
		else
		{
			//Format conversion
			IWICFormatConverter* FC;
			hr = pWIC->CreateFormatConverter(&FC);
			assert(SUCCEEDED(hr));
			hr = FC->Initialize(scaler, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
			assert(SUCCEEDED(hr));
			hr = FC->CopyPixels(0, uiRowPitch, uiImageSize, tUsableImage.pPixelData);
			assert(SUCCEEDED(hr));

			ReleaseCOM(FC);
		}

		ReleaseCOM(scaler);
	}
	else
	{
		// Format conversion but no resize
		IWICFormatConverter* FC;
		hr = pWIC->CreateFormatConverter(&FC);
		assert(SUCCEEDED(hr));
		hr = FC->Initialize(pBitmapFrame, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		assert(SUCCEEDED(hr));
		hr = FC->CopyPixels(0, uiRowPitch, uiImageSize, tUsableImage.pPixelData);
		assert(SUCCEEDED(hr));

		ReleaseCOM(FC);
	}

	//Return the image data
	return(tUsableImage);
}
