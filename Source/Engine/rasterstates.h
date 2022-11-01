#pragma once
#ifndef __RASTER_STATES_H__
#define __RASTER_STATES_H__

//Local Includes
#include "dxcommon.h"

namespace DefaultRasterStates
{
	//Enums
	enum ERasterStates
	{
		RS_SOLID,
		RS_WIREFRAME,

		MAX_RS
	};

	static D3D11_RASTERIZER_DESC gRasterStates[MAX_RS] =
	{
		//RS_SOLID
		{
			D3D11_FILL_SOLID,	//FillMode
			D3D11_CULL_BACK,	//CullMode
			false,				//FrontCounterClockwise
			0,					//DepthBias
			0.0f,				//DepthBiasClamp
			0.0f,				//SlopeScaledDepthBias
			true,				//DepthClipEnable
			false,				//ScissorEnable
			true,				//MultisampleEnable
			false,				//AntialiasedLineEnable
		},

		//RS_WIREFRAME
		{
			D3D11_FILL_WIREFRAME,	//FillMode
			D3D11_CULL_NONE,		//CullMode
			false,					//FrontCounterClockwise
			0,						//DepthBias
			0.0f,					//DepthBiasClamp
			0.0f,					//SlopeScaledDepthBias
			true,					//DepthClipEnable
			false,					//ScissorEnable
			true,					//MultisampleEnable
			false,					//AntialiasedLineEnable
		}
	};
}

#endif //__RASTER_STATES_H__