#pragma once
#ifndef __BLEND_STATES_H__
#define __BLEND_STATES_H__

//Local Includes
#include "dxcommon.h"

namespace DefaultBlendStates
{
	//Enums
	enum EBlendStates
	{
		BS_DEFAULT,

		MAX_BS
	};

	static const D3D11_RENDER_TARGET_BLEND_DESC gDefaultRTBlendDesc =
	{
		true,						//BOOL BlendEnable;
		D3D11_BLEND_SRC_ALPHA,		//D3D11_BLEND SrcBlend;
		D3D11_BLEND_INV_SRC_ALPHA,	//D3D11_BLEND DestBlend;
		D3D11_BLEND_OP_ADD,			//D3D11_BLEND_OP BlendOp;
		D3D11_BLEND_ONE,			//D3D11_BLEND SrcBlendAlpha;
		D3D11_BLEND_ZERO,			//D3D11_BLEND DestBlendAlpha;
		D3D11_BLEND_OP_ADD,			//D3D11_BLEND_OP BlendOpAlpha;
		0x0f,						//UINT8 RenderTargetWriteMask;
	};

	static const D3D11_BLEND_DESC gBlendStates[MAX_BS] =
	{
		//BS_DEFAULT
		{
			true,	//AlphaToCoverageEnable
			false,	//IndependentBlendEnable

			//D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[8];
			{
				gDefaultRTBlendDesc, //[0]
				gDefaultRTBlendDesc, //[1]
				gDefaultRTBlendDesc, //[2]
				gDefaultRTBlendDesc, //[3]
				gDefaultRTBlendDesc, //[4]
				gDefaultRTBlendDesc, //[5]
				gDefaultRTBlendDesc, //[6]
				gDefaultRTBlendDesc  //[7]
			}

		},
	};
}

#endif //__BLEND_STATES_H__