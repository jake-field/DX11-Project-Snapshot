#pragma once
#ifndef __STATIC_MESH_INSTANCER_H__
#define __STATIC_MESH_INSTANCER_H__

//Local Include
#include "instancepool.hpp"

//Types
struct TStaticMeshInstance
{
	float3 pos;
	float3 scale;
	float4 rot;
};

//Prototypes
class CRenderer;
class CStaticMesh;
class CStaticMeshInstancer //TODO: Consider making this an IEntity for Draw/Process of everything in the batch
{
	//Member Functions
public:
	CStaticMeshInstancer();
	~CStaticMeshInstancer();

	bool Initialize(CRenderer* _pRenderer, unsigned int _uiMaxInstanceCount);

	//Do we offer the choice of appending to a batch that persists between frames? If so we need a vector
	//If not, AddToBatch() will empty the instance batch when called on a new frame
	//DrawBatch() locks the pool and draws. If AddToBatch() isn't called by next DrawBatch(), the previous instance data will be drawn
	//	allowing a static instance grouping, such as a city
	bool ReadyBatch(bool _bAppendToLastFrame = false); //Unlocks instance buffer, if _keeplast then copy last frame (instancepool supports this natively)
	bool AddToBatch(CStaticMesh* _pMesh); //Adds a mesh to the instance pool in append mode, may start at 0 if readybatch(false)
	void FinishBatch(); //Close batch
	bool DrawBatch(); //Closes? batch and draws



	//Member Variables
protected:
	CInstancePool<TStaticMeshInstance>* m_pInstancePool;
	CStaticMesh* m_pReferenceMesh;

};

#endif //__STATIC_MESH_INSTANCER_H__
