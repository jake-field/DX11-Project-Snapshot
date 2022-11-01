#pragma once
#ifndef __IENTITY_H__
#define __IENTITY_H__

//Prototype
class IEntity
{
	//Member Functions
public:
	virtual void Process(float _fDeltaTick) = 0;
	virtual void Draw() = 0;

};

#endif //__IENTITY_H__