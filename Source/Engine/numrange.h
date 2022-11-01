#pragma once
#ifndef __RANGE_H__
#define __RANGE_H__

//Provides a returnable storage class for two values
//	- Test(_val); returns true if the _val fits between <a, b>
template<class T>
struct TRange
{
	//Variables
	T a, b;

	//Functions
	TRange()
		: a(0)
		, b(0)
	{
		//Constructor
	}

	TRange(T _a, T _b)
		: a(_a)
		, b(_b)
	{
		//Constructor
	}

	//Tests that _val is within <a, b>
	bool Check(T _val)
	{
		bool bValid = false;

		if(a < b)
		{
			//Positive Range
			bValid = (_val >= a && _val <= b);
		}
		else if(a > b)
		{
			//Negative Range
			bValid = (_val <= a && _val >= b);
		}
		else
		{
			//A==B
			bValid = (_val == a);
		}

		return(bValid);
	}
};

#endif //__RANGE_H__
