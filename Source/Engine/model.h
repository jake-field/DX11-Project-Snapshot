#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

//Library Includes
#include <vector>
#include <string>
#include <assimp\matrix4x4.h>

//Local Includes
#include "vertexdefs.h"
#include "mesh.hpp"
#include "asset.h"

//Types
struct TModelMeshInstance
{
	unsigned int uiMeshID;
	float4x4 matObject;
	float4x4 matRotation;
	float3 vec3Pos;
	float3 vec3Scale;
	float3 vec3Rot;
};

//Prototype
struct aiNode;
class CModel: public IAsset
{
	//Memeber Functions
protected:
	CModel();
	CModel(const CModel& _rhs) = default;
	virtual ~CModel();

public:
	IMesh* GetMeshObject(unsigned int _uiIndex) const;
	unsigned int GetMeshCount() const;

	TModelMeshInstance GetInstance(unsigned int _uiIndex) const;
	unsigned int GetInstanceCount() const;

	void SetMaterial(int _iMatID, const TMaterial& _rtMaterial); //rename to material slot
	int GetMaterialCount() const;

	void GetSkeleton(int _iMeshIndex); //Return skeleton pointer if there is one
	bool IsRigged() const; //same as checking GetSkeleton != nullptr

	static EAssetType GetAssetType();

protected:
	bool Load(const char* _kpcFilename);
	void Release();

	void ProcessSceneNodes(aiNode* _pNode, const float3* _vec3UpRightFwd, float3 _vec3PosScaleRot[3]);

	//Member Variables
protected:
	std::vector<CMesh<TVertexTexNorm>*> m_vecMeshes;
	std::vector<TModelMeshInstance> m_vecInstances;
	int m_iMaterialCount;

	friend CAssetManager;
};

#endif //__MODEL_H__