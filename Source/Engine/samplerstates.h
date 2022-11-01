#pragma once
#ifndef __SAMPLER_STATES_H__
#define __SAMPLER_STATES_H__

//Local Includes
#include "dxcommon.h"
#include "shaderglobals.h"

namespace DefaultSamplerStates
{
	//Enums
	enum ESamplerStates
	{
		SS_ANISO,
		SS_SHADOW,

		MAX_SS,
		START_SLOT = ShaderGlobals::ERegisters::SAMP_ANISO //Adjust start slot based on location in shader globals
	};

	static D3D11_SAMPLER_DESC gSamplerStates[MAX_SS] = 
	{
		//SS_ANISO
		{
			D3D11_FILTER_ANISOTROPIC,		//Filter
			D3D11_TEXTURE_ADDRESS_MIRROR,	//AddressU
			D3D11_TEXTURE_ADDRESS_MIRROR,	//AddressV
			D3D11_TEXTURE_ADDRESS_MIRROR,	//AddressW
			0.0f,							//MipLODBias
			2,								//MaxAnisotropy
			D3D11_COMPARISON_NEVER,			//ComparisonFunc
			{1.0f, 0.0f, 0.5f, 1.0f},		//BorderColour[4]
			0.0f,							//MinLOD
			FLT_MAX,						//MaxLOD
		},

		//SS_SHADOW
		{
			D3D11_FILTER_ANISOTROPIC,		//Filter
			D3D11_TEXTURE_ADDRESS_BORDER,	//AddressU
			D3D11_TEXTURE_ADDRESS_BORDER,	//AddressV
			D3D11_TEXTURE_ADDRESS_BORDER,	//AddressW
			0.0f,							//MipLODBias
			2,								//MaxAnisotropy
			D3D11_COMPARISON_NEVER,			//ComparisonFunc
			{1.0f, 1.0f, 1.0f, 1.0f},		//BorderColour[4] //White to mask the fact that shadows don't cover entire scene
			0.0f,							//MinLOD
			FLT_MAX,						//MaxLOD
		}
	};
}

#endif //__SAMPLER_STATES_H__