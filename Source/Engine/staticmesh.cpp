//Local Includes
#include "camera.h"
#include "model.h"
#include "staticmeshinstancer.h"

//This Include
#include "staticmesh.h"

//Implementation
CStaticMesh::CStaticMesh()
	: m_pModel(nullptr)
	, m_pMesh(nullptr)
	, m_pInstancer(nullptr)
	, m_iMeshID(-1)
	, m_bVisible(true)
{
	//Constructor
}

CStaticMesh::~CStaticMesh()
{
	//Destructor
	m_pModel = nullptr;
	m_pMesh = nullptr;
	m_pInstancer = nullptr;
	m_bVisible = false;
}

bool
CStaticMesh::Initialize(CModel* _pModel, int _iInstanceID, CStaticMeshInstancer* _pInstancer)
{
	//TODO: Error/pointer checking
	//TODO: This class needs an overhaul for dealing with instanced versions
	 
	
	//Check asset state of model before building combined AABB
	//Blocking but not much we can do...
	while(_pModel->GetAssetState() == EAssetState::Queued || _pModel->GetAssetState() == EAssetState::Loading);

	//Bind model
	m_pModel = _pModel;
	m_pInstancer = _pInstancer;

	//Bounding box generation
	float3 vec3Min(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 vec3Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for(unsigned int i = 0; i < _pModel->GetInstanceCount(); ++i)
	{
		//Force use of a single instance
		//Placed here because it's dirty to have this in the for loop conditional
		if(_iInstanceID != -1) i = _iInstanceID;

		TModelMeshInstance tInstance = _pModel->GetInstance(i);
		IMesh* pMesh = _pModel->GetMeshObject(tInstance.uiMeshID);

		//Adjust mesh bounding box to instance pos/size
		float3 vec3MeshCenter = (float3)pMesh->GetBoundingBox().Center;
		float3 vec3MeshExtends = (float3)pMesh->GetBoundingBox().Extents;
		float3 vec3MeshMin = vec3MeshCenter - vec3MeshExtends;
		float3 vec3MeshMax = vec3MeshCenter + vec3MeshExtends;

		//Bounding box min/max point calculation
		vec3Min.x = min(vec3Min.x, vec3MeshMin.x);
		vec3Min.y = min(vec3Min.y, vec3MeshMin.y);
		vec3Min.z = min(vec3Min.z, vec3MeshMin.z);

		vec3Max.x = max(vec3Max.x, vec3MeshMax.x);
		vec3Max.y = max(vec3Max.y, vec3MeshMax.y);
		vec3Max.z = max(vec3Max.z, vec3MeshMax.z);

		//Make new bounding box
		m_tOBB.Center = (vec3Min + vec3Max) * 0.5f;
		m_tOBB.Extents = (vec3Max - vec3Min) * 0.5f;
		m_tOriginalOBB = m_tOBB; //TODO: This is redundant code, consider Oriented BB and BB

		//Single instance breakout
		if(_iInstanceID != -1 || !_pInstancer && _pModel->GetInstanceCount() <= 1)
		{
			//Store quick access to instance mesh
			m_pMesh = pMesh;
			m_iMeshID = tInstance.uiMeshID;

			//We're a single instance, set our transform to that of the instance
			SetRotation(m_pModel->GetInstance(i).vec3Rot);
			SetPosition(m_pModel->GetInstance(i).vec3Pos);
			SetScale(m_pModel->GetInstance(i).vec3Scale);

			break;
		}
	}

	//Bounding Sphere gen
	DirectX::BoundingSphere::CreateFromBoundingBox(m_tBoundingSphere, m_tOBB);

	//Render options
	SetRenderOptions(true, true, true);

	if(_iInstanceID != -1)
	{
		auto id = m_pModel->GetInstance(_iInstanceID).uiMeshID;
		auto mat = m_pModel->GetMeshObject(id)->GetMaterial();
		SetRenderOptions(true, mat.bCastShadow, mat.bReceiveShadow);
	}

	//TODO: Fix this
	return(true);
}

void
CStaticMesh::Draw()
{
	if(m_pModel)
	{
		//Single instance, it's a little bit different to draw single ones due to init
		if(m_pMesh)
		{
			//We cannot use this here as it bogs the engine draw logic down with map/unmap which technically shouldn't even work
			//if(m_pInstancer) m_pInstancer->AddToBatch(this);
			if(!m_pInstancer) m_pMesh->Draw(&m_matWorld);
		}
		else
		{
			//Render instances rather than individual meshes
			for(unsigned int i = 0; i < m_pModel->GetInstanceCount(); ++i)
			{
				IMesh* pMesh = m_pModel->GetMeshObject(m_pModel->GetInstance(i).uiMeshID);

				float4x4 matObject = m_pModel->GetInstance(i).matObject;
				XMMATRIX xmmatObject = XMLoadFloat4x4(&matObject);
				XMMATRIX xmmatWorld = XMLoadFloat4x4(&m_matWorld);

				XMMATRIX xmmatNew = XMMatrixMultiply(xmmatObject, xmmatWorld);

				float4x4 matWorld;
				XMStoreFloat4x4(&matWorld, xmmatNew);

				//TODO: Should we consider supporting instancing here? flexibility waivers due to multi-mesh etc.
				//		and by doing this, the code here will be stupid to account for multiple instancers, one per mesh
				//		unless we adjust the instancer such that we can draw 0,n for one mesh, then n through y for another mesh
				//		Doing that would require sorting and a lookup
				pMesh->Draw(&matWorld);
			}
		}
	}
}

int
CStaticMesh::GetTriangleCount() const
{
	int iTriangles = 0;

	//If we're a specific mesh of the model
	if(m_pMesh)
	{
		int iVertices = m_pMesh->GetVertexCount();
		int iIndices = m_pMesh->GetIndexCount();
		iTriangles = (iIndices > 0 ? iIndices : iVertices) / 3;
	}
	else
	{
		//We are the whole model, not a specific mesh in the model
		//We could just check mesh here but considering Assimp most likely will force instances, count for each instance instead of mesh
		for(unsigned int i = 0; i < m_pModel->GetInstanceCount(); ++i)
		{
			int iMeshID = m_pModel->GetInstance(i).uiMeshID;
			int iVertices = m_pModel->GetMeshObject(iMeshID)->GetVertexCount();
			int iIndices = m_pModel->GetMeshObject(iMeshID)->GetIndexCount();
			iTriangles += (iIndices > 0 ? iIndices : iVertices) / 3;
		}
	}

	return(iTriangles);
}

int CStaticMesh::GetMeshID() const
{
	return(m_iMeshID);
}
