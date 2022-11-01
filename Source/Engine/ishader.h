#pragma once
#ifndef __ISHADER_H__
#define __ISHADER_H__

//Local Includes
#include "types.h"

//Prototypes
class IMesh;
class IShader
{
	//Member Functions
public:
	virtual bool ApplyShader(int _iPass = 0) = 0;
	virtual bool SetPass(int _iPass) = 0;
	virtual void FinishShader() = 0;

	virtual bool Predraw(const IMesh* _pMesh, const float4x4* _ptWorldMatrix, bool _bInstanced = false) = 0;

	//TODO: Consider why this is here and not in DX11Shader
	static IShader* GetActiveShader()
	{
		return(sm_pActiveShader);
	};

protected:
	static IShader* sm_pActiveShader;

};

#endif //__ISHADER_H__
