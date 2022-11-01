#pragma once
#ifndef __ARMATURE_H__
#define __ARMATURE_H__

//Local Includes
#include "types.h"

//Types
struct TBone
{
	//Variables
	const char strName;
	int iNameLen;

	TBone* pParent;
	TBone** pChildren;
	int iChildCount;

	float4x4 matLocalOffset; //heirarchy offset
	float4x4 matOffset;	//root offset
	
	//Functions
};

//Prototype
class CArmature
{
	//Member Functions
public:
	CArmature();
	~CArmature();

	bool Initialize(void* _pBones, void* _pRootNode);

	TBone* GetRoot() const;
	TBone* GetBoneByName(const char* _pcBoneName) const;
	TBone* GetBoneByID(unsigned int _uiID) const;
	unsigned int GetBoneCount() const;


	//Member Variables
protected:
	TBone* m_pBones;
	int m_iBoneCount;
};

#endif //__ARMATURE_H__
