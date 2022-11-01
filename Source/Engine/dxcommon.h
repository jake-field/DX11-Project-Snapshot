#pragma once
#ifndef __DXCOMMON_H__
#define __DXCOMMON_H__

//D3D Debug defines
#if defined(EH_DEBUG) || defined(_DEBUG)
	#ifndef D3D_DEBUG_INFO
		#define D3D_DEBUG_INFO
	#endif //D3D_DEBUG_INFO
#endif //_DEBUG

//Preprocessor
#define ReleaseCOM(x) {if(x){x->Release(); x = nullptr;}}

//Library Includes
#include <d3d11.h>
#include <directxmath.h>

//Namespace
using namespace DirectX;

//Helper functions
//Loose comparison
static bool floatCompare(float _a, float _b, float _tolerance = 0.0005f) //Tolerance 0.0005f either side, can shift 0.0005f to 0.001f or 0.0f
{
	bool bIsEqual = (_a == _b);

	if(!bIsEqual)
	{
		float al = _a - _tolerance;
		float ah = _a + _tolerance;
		float bl = _b - _tolerance;
		float bh = _b + _tolerance;

		bool a = (_b < ah) && (_b > al);
		bool b = (_a < bh) && (_a > bl);

		bIsEqual = (a || b);
	}

	return(bIsEqual);
}

#endif //__DXCOMMON_H__
