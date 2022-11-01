#pragma once
#ifndef __MATERIAL_H__
#define __MATERIAL_H__

//Local Includes
#include "texture.h"

struct TMaterial
{
	//Variables
	//TODO: support colors, methods or specific shader settings?
	CTexture* pDiffuseTex;
	CTexture* pNormalTex;
	CTexture* pSpecularTex;
	CTexture* pAOTex;
	bool bCastShadow;
	bool bReceiveShadow;
	bool bTransparent;

	//Functions
	TMaterial()
		: pDiffuseTex(nullptr)
		, pNormalTex(nullptr)
		, pSpecularTex(nullptr)
		, pAOTex(nullptr)
		, bCastShadow(true)
		, bReceiveShadow(true)
		, bTransparent(false)
	{
		//Constructor
	}

	TMaterial(CTexture* _pDiffuseTex, CTexture* _pNormalTex = nullptr, CTexture* _pSpecularTex = nullptr, CTexture* _pAOTex = nullptr)
		: pDiffuseTex(_pDiffuseTex)
		, pNormalTex(_pNormalTex)
		, pSpecularTex(_pSpecularTex)
		, pAOTex(_pAOTex)
		, bCastShadow(true)
		, bReceiveShadow(true)
		, bTransparent(false)
	{
		//Constructor
	}
};

#endif //__MATERIAL_H__