#pragma once
#ifndef __DX11_SHADER_H__
#define __DX11_SHADER_H__

//Library Includes
#include <vector>

//Local Includes
#include "dxcommon.h"
#include "ishader.h"

//Types
enum class EShaderType
{
	VERTEX,
	PIXEL,
	GEOMETRY,

	//Following not implemented
	//DOMAIN,
	//HULL,
	//COMPUTE
};

//Data Types
struct TVertexLayoutSemantic
{
	//Variables
	LPCSTR strName;
	int iSemanticIndex;
	DXGI_FORMAT eFormat;
	int iInputSlot;
	int iInstanceStepRate; //If -1 then non-instanced

	//Functions
	TVertexLayoutSemantic(LPCSTR _strName, DXGI_FORMAT _eFormat, int _iSemanticIndex = 0, int _iInputSlot = 0, int _iInstanceStepRate = -1)
		: strName(_strName)
		, iSemanticIndex(_iSemanticIndex)
		, eFormat(_eFormat)
		, iInputSlot(_iInputSlot)
		, iInstanceStepRate(_iInstanceStepRate)
	{
		//Constructor
	}
};

struct TShaderFileDesc
{
	//Variables
	const char* strFilename;

	//Settings for compiling shaders
	bool bUncompiled;
	const char* strVersion;
	const char* strEntryPoint;

	//Functions
	//Blank constructor
	TShaderFileDesc()
		: strFilename(nullptr)
		, strVersion(nullptr)
		, strEntryPoint(nullptr)
		, bUncompiled(false)
	{
		//Constructor
	}

	//Compiled HLSL constructor (*.cso)
	TShaderFileDesc(const char* _strFilename)
		: strFilename(_strFilename)
		, strVersion(nullptr)
		, strEntryPoint(nullptr)
		, bUncompiled(false)
	{
		//Constructor
	}

	//Uncompiled HLSL constructor (*.hlsl)
	TShaderFileDesc(const char* _strFilename, const char* _strVersion, const char* _strEntryPoint)
		: strFilename(_strFilename)
		, strVersion(_strVersion)
		, strEntryPoint(_strEntryPoint)
		, bUncompiled(true)
	{
		//Constructor
	}
};

struct TShaderPass
{
	//Variables
	ID3D11VertexShader*		pVertexShader;
	ID3D11PixelShader*		pPixelShader;
	ID3D11GeometryShader*	pGeometryShader;
	ID3D11DomainShader*		pDomainShader;
	ID3D11HullShader*		pHullShader;
	ID3D11ComputeShader*	pComputeShader;
	ID3D11InputLayout*		pVertexLayout;

	//Functions
	TShaderPass()
		: pVertexShader(nullptr)
		, pPixelShader(nullptr)
		, pGeometryShader(nullptr)
		, pDomainShader(nullptr)
		, pHullShader(nullptr)
		, pComputeShader(nullptr)
		, pVertexLayout(nullptr)
	{
		//Constructor
	}

	void Release()
	{
		ReleaseCOM(pVertexShader);
		ReleaseCOM(pPixelShader);
		ReleaseCOM(pGeometryShader);
		ReleaseCOM(pDomainShader);
		ReleaseCOM(pHullShader);
		ReleaseCOM(pComputeShader);
		ReleaseCOM(pVertexLayout);
	}
};

//Prototypes
class CRenderer;
class IMesh;
class CDX11Shader: public IShader
{
	//Member Functions
public:
	CDX11Shader();
	virtual ~CDX11Shader();
	virtual bool Initialize(CRenderer* _pRenderer) = 0;

	//Apply the shader and set it as active for the scene
	//TODO: maybe remove _iPass from ApplyShader and make Passes entirely optional
	virtual bool ApplyShader(int _iPass = 0); //When called, Sets Pass to pass 0 by default
	virtual bool SetPass(int _iPass) = 0; //Set current pass
	virtual void FinishShader() = 0; //OPT: Called when ApplyShader() is called to cleanup bound resources

	virtual bool Predraw(const IMesh* _pMesh, const float4x4* _ptWorldMatrix, bool _bInstanced = false) = 0;

protected:
	bool LoadFromFile(EShaderType _eShaderSlot, int _iPass, TShaderFileDesc _tDesc, TVertexLayoutSemantic* _ptSemantics = nullptr, int _iSemanticCount = 0);

	//TODO: Load from the Resource file?
	//bool LoadFromMemory(EShaderType _eShaderSlot, int _iPass, void* _pData, TVertexLayoutSemantic* _ptSemantics = nullptr, int _iSemanticCount = 0);

private:
	bool CreateShader(EShaderType _eShaderSlot, int _iPass, LPVOID _shaderBuffer, SIZE_T _shaderSize, TVertexLayoutSemantic* _ptSemantics = nullptr, int _iSemanticCount = 0);

	//Member Variables
protected:
	CRenderer* m_pRenderer;
	std::vector<TShaderPass> m_vecPasses; //TODO: Consider not using a vector here, a linked list would be fine

};

#endif //__DX11_SHADER_H__