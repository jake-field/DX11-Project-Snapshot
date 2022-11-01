#pragma once
#ifndef __STATIC_MESH_H__
#define __STATIC_MESH_H__

//Local Includes
#include "entity3d.h"

//Prototype
class IMesh;
class CModel;
class CStaticMeshInstancer;
class CStaticMesh: public CEntity3D
{
	//Member Functions
public:
	CStaticMesh();
	virtual ~CStaticMesh();

	virtual bool Initialize(CModel* _pModel, int _iInstanceID = -1, CStaticMeshInstancer* _pInstancer = nullptr);

	//If m_pInstancer, draw will silently fail as the Instancer will handle the drawing of this mesh
	virtual void Draw();

	//Just for debug purposes
	virtual int GetTriangleCount() const;
	virtual int GetMeshID() const;

	//Member Variables
protected:
	CStaticMeshInstancer* m_pInstancer; //Instancer, if valid then Draw() only adds to the instancer batch
	CModel* m_pModel; //Main model for this static mesh, this should be a shared_pointer
	IMesh* m_pMesh; //Obtained from pModel if Init(model, !0)
	int m_iMeshID;
	bool m_bVisible;

	friend CStaticMeshInstancer;

};

#endif //__STATIC_MESH_H__
