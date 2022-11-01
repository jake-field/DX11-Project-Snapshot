#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

//Debug defines
#if defined(_DEBUG)
//Built in memoryleak assistant
#pragma warning(push)
#pragma warning(disable: 4005) //crtdbg fires off _malloca redef
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(pop)
#endif

//Library Includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

//Local Includes
#include "types.h"
#include "numrange.h"

//Preprocessor
#define SafeDelete(x) {if(x){delete x; x = nullptr;}}
#define SafeDeleteArray(x) {if(x){delete[] x; x = nullptr;}}
#define SizeofArray(x) sizeof(x) / sizeof(x[0])

inline float randf()
{
	//Returns a random float between 0.0f and 1.0f
	return(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

inline float randf(float a, float b) //Random float value between A, B
{
	return(a + randf() * (b - a));
}

template<class T>
inline bool ClampValue(T& _value, T _min, T _max)
{
	//Clamp a value to a min and max
	if(_value < _min) _value = _min;
	else if(_value > _max) _value = _max;
	else return(false); //Untouched
	return(true); //Touched
};

#endif // __COMMON_H__