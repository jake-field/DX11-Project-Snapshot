//Local Includes
#include "instancepool.hpp"
#include "staticmesh.h"
#include "mesh.hpp"

//This Include
#include "staticmeshinstancer.h"

//Implementation
CStaticMeshInstancer::CStaticMeshInstancer()
	: m_pInstancePool(nullptr)
	, m_pReferenceMesh(nullptr)
{
	//Constructor
}

CStaticMeshInstancer::~CStaticMeshInstancer()
{
	//Destructor
	SafeDelete(m_pInstancePool);
	m_pReferenceMesh = nullptr;
}

bool
CStaticMeshInstancer::Initialize(CRenderer* _pRenderer, unsigned int _uiMaxInstanceCount)
{
	bool bSuccess = false;

	m_pInstancePool = new CInstancePool<TStaticMeshInstance>;
	if(m_pInstancePool) bSuccess = m_pInstancePool->Initialize(_pRenderer, nullptr, _uiMaxInstanceCount, true);

	return(bSuccess);
}

bool
CStaticMeshInstancer::ReadyBatch(bool _bAppendToLastFrame)
{
	return(m_pInstancePool != nullptr && m_pInstancePool->Unlock(!_bAppendToLastFrame));
}

bool
CStaticMeshInstancer::AddToBatch(CStaticMesh* _pMesh)
{
	bool bSuccess = false;
	
	if(m_pInstancePool && _pMesh && _pMesh->m_pInstancer == this)
	{
		m_pReferenceMesh = _pMesh; //Used for pulling the mesh

		TStaticMeshInstance tInstanceData;
		float3 vec3Euler = _pMesh->GetRotation();
		float4 vec4Quat;

		XMStoreFloat4(&vec4Quat, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(vec3Euler.x), XMConvertToRadians(vec3Euler.y), XMConvertToRadians(vec3Euler.z)));

		tInstanceData.pos = _pMesh->GetPosition();
		tInstanceData.scale = _pMesh->GetScale();
		tInstanceData.rot = vec4Quat;

		bSuccess = m_pInstancePool->AppendInstances(&tInstanceData, 1);
	}

	return(bSuccess);
}

void
CStaticMeshInstancer::FinishBatch()
{
	if(m_pInstancePool) m_pInstancePool->Lock();
}

bool
CStaticMeshInstancer::DrawBatch()
{
	FinishBatch();

	if(m_pInstancePool && m_pReferenceMesh && m_pReferenceMesh->m_pMesh)
	{
		m_pReferenceMesh->m_pMesh->DrawInstanced(m_pInstancePool, {0, m_pInstancePool->GetValid()});
	}

	return(false);
}
