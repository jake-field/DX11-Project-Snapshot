//Library Includes
#include <d3dcompiler.h>
#include <istream>

//Local Includes
#include "renderer.h"

//This Include
#include "dx11shader.h"

//Static Variables
IShader* IShader::sm_pActiveShader = nullptr;

//Implementation
CDX11Shader::CDX11Shader()
	: m_pRenderer(nullptr)
{
	//Constructor
}

CDX11Shader::~CDX11Shader()
{
	//Prevent possible exception 
	//TODO: Maybe call FinishShader() here?
	if(sm_pActiveShader == this) sm_pActiveShader = nullptr;

	//Release all shader pass COMs
	for(unsigned int i = 0; i < m_vecPasses.size(); ++i) m_vecPasses[i].Release();
	m_vecPasses.clear();
}

bool
CDX11Shader::ApplyShader(int _iPass)
{
	bool bAlreadySet = true;

	//Save time by skipping ready if we were the last shader to ready up
	if(sm_pActiveShader != this)
	{
		//Attempt cleanup of last shader
		if(sm_pActiveShader) sm_pActiveShader->FinishShader();

		//Bind active to this
		sm_pActiveShader = this;
		bAlreadySet = false;
	}

	//Return if apply was actually done
	return(bAlreadySet);
}

bool
CDX11Shader::SetPass(int _iPass)
{
	//_iPass within bounds of valid passes
	bool bSuccessful = (_iPass >= 0 && _iPass < (int)m_vecPasses.size());

	//_iPass is valid and the shader has already been applied
	if(bSuccessful && sm_pActiveShader == this)
	{
		//Bind Shaders (Shaders set to NULL will be disabled)
		m_pRenderer->GetDeviceContext()->VSSetShader(m_vecPasses[_iPass].pVertexShader, nullptr, 0);
		m_pRenderer->GetDeviceContext()->PSSetShader(m_vecPasses[_iPass].pPixelShader, nullptr, 0);
		m_pRenderer->GetDeviceContext()->GSSetShader(m_vecPasses[_iPass].pGeometryShader, nullptr, 0);
		m_pRenderer->GetDeviceContext()->HSSetShader(m_vecPasses[_iPass].pHullShader, nullptr, 0);
		m_pRenderer->GetDeviceContext()->DSSetShader(m_vecPasses[_iPass].pDomainShader, nullptr, 0);
		m_pRenderer->GetDeviceContext()->CSSetShader(m_vecPasses[_iPass].pComputeShader, nullptr, 0); //TODO: Future support for multiple Compute Shaders

		//Nothing fancy, consider this a success
		bSuccessful = true;
	}
	else
	{
		//Shader not active
		bSuccessful = false;
	}

	return(bSuccessful);
}

void
CDX11Shader::FinishShader()
{
	//Prevent duplicate calls
	sm_pActiveShader = nullptr;
}

bool
CDX11Shader::LoadFromFile(EShaderType _eShaderSlot, int _iPass, TShaderFileDesc _tDesc, TVertexLayoutSemantic* _ptSemantics, int _iSemanticCount)
{
	size_t len;
	char* bytes = nullptr;
	FILE* theFile = nullptr;
	bool bSuccessful = false;

	//Open the file as binary if the shader file is compiled, otherwise ascii
	fopen_s(&theFile, _tDesc.strFilename, _tDesc.bUncompiled ? "r" : "rb");

	//File open?
	if(theFile != nullptr)
	{
		//Get the length of the file and allocate memory for the contents
		fseek(theFile, 0, SEEK_END);
		len = ftell(theFile);
		bytes = (char*)malloc(len);
		fseek(theFile, 0, SEEK_SET);

		//Memory allocation succeeded
		if(bytes != nullptr)
		{
			//Read the file into memory
			fread(bytes, 1, len, theFile);

			//Compile?
			if(_tDesc.bUncompiled)
			{
				ID3DBlob* pShaderBlob = nullptr;
				unsigned int uiFlags = D3DCOMPILE_ENABLE_STRICTNESS;

				//Compile with debug flags if we're debugging
				#ifdef _DEBUG
				uiFlags |= D3DCOMPILE_DEBUG;
				#endif //_DEBUG

				//Attempt to compile the shader
				HRESULT hResult = D3DCompile(bytes, len, nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, _tDesc.strEntryPoint, _tDesc.strVersion, uiFlags, 0, &pShaderBlob, nullptr);

				//Attempt to create the shader if it compiled successfully
				bSuccessful = SUCCEEDED(hResult) && CreateShader(_eShaderSlot, _iPass, pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), _ptSemantics, _iSemanticCount);

				//Release the blob if it was created
				ReleaseCOM(pShaderBlob);
			}
			else
			{
				//Shader file is compiled
				bSuccessful = CreateShader(_eShaderSlot, _iPass, bytes, len, _ptSemantics, _iSemanticCount);
			}

			free(bytes);
		}

		fclose(theFile);
	}

	return(bSuccessful);
}

bool
CDX11Shader::CreateShader(EShaderType _eShaderSlot, int _iPass, LPVOID _shaderBuffer, SIZE_T _shaderSize, TVertexLayoutSemantic* _ptSemantics, int _iSemanticCount)
{
	bool bSuccessful = false;

	//Make sure the pass vector scales to support new passes
	while((int)m_vecPasses.size() <= _iPass) m_vecPasses.push_back(TShaderPass());

	//Per type creation
	switch(_eShaderSlot)
	{
		case EShaderType::VERTEX:
			ReleaseCOM(m_vecPasses[_iPass].pVertexShader); //In case of recreation
			bSuccessful = SUCCEEDED(m_pRenderer->GetDevice()->CreateVertexShader(_shaderBuffer, _shaderSize, nullptr, &m_vecPasses[_iPass].pVertexShader));
			break;

		case EShaderType::GEOMETRY:
			ReleaseCOM(m_vecPasses[_iPass].pGeometryShader); //In case of recreation
			bSuccessful = SUCCEEDED(m_pRenderer->GetDevice()->CreateGeometryShader(_shaderBuffer, _shaderSize, nullptr, &m_vecPasses[_iPass].pGeometryShader));
			break;

		case EShaderType::PIXEL:
			ReleaseCOM(m_vecPasses[_iPass].pPixelShader); //In case of recreation
			bSuccessful = SUCCEEDED(m_pRenderer->GetDevice()->CreatePixelShader(_shaderBuffer, _shaderSize, nullptr, &m_vecPasses[_iPass].pPixelShader));
			break;

		default:
			#ifdef _DEBUG
			DebugBreak(); //TODO: Add the remaining shader types
			#endif //_DEBUG
			break;
	}

	//Shifted here for cleaner code instead of cluttering the switch
	//Vertex shaders require an Input Layout to be created with them
	if(bSuccessful && _eShaderSlot == EShaderType::VERTEX)
	{
		//Valid semantics?
		if(_ptSemantics != nullptr && _iSemanticCount > 0)
		{
			D3D11_INPUT_ELEMENT_DESC* pLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[_iSemanticCount];
			ZeroMemory(pLayoutDesc, sizeof(D3D11_INPUT_ELEMENT_DESC) * _iSemanticCount);
			ReleaseCOM(m_vecPasses[_iPass].pVertexLayout); //In case of recreation

			//Build layout
			for(int i = 0; i < _iSemanticCount; ++i)
			{
				pLayoutDesc[i].SemanticName = _ptSemantics[i].strName;
				pLayoutDesc[i].SemanticIndex = _ptSemantics[i].iSemanticIndex;
				pLayoutDesc[i].Format = _ptSemantics[i].eFormat;

				//If there is instance step rates, assume this is an instanced semantic
				pLayoutDesc[i].InputSlot = _ptSemantics[i].iInputSlot;
				pLayoutDesc[i].InputSlotClass = _ptSemantics[i].iInstanceStepRate == -1 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
				if(_ptSemantics[i].iInstanceStepRate != -1) pLayoutDesc[i].InstanceDataStepRate = _ptSemantics[i].iInstanceStepRate;

				//First Instanced requires 0. So if first 3 are normal then 4th is instanced, 4th semantic resets ABOffset to 0, then following has append
				if(i && !(_ptSemantics[i - 1].iInstanceStepRate == -1 && _ptSemantics[i].iInstanceStepRate != -1))
				{
					pLayoutDesc[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				}
			}

			//Attempt to create the vertex input layout.
			bSuccessful = SUCCEEDED(m_pRenderer->GetDevice()->CreateInputLayout(pLayoutDesc, _iSemanticCount, _shaderBuffer, _shaderSize, &m_vecPasses[_iPass].pVertexLayout));

			//Release the layout description
			SafeDeleteArray(pLayoutDesc);
		}
		else
		{
			//Failed to create semantics due to invalid inputs
			bSuccessful = false;
		}

		//If we failed to create the input layout, release shader
		if(!bSuccessful) ReleaseCOM(m_vecPasses[_iPass].pVertexShader);
	}

	//Returns true if the shader was created successfully, as well as the Input Layout if the shader was a Vertex Shader
	return(bSuccessful);
}
