//Library Includes
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\matrix4x4.h>

//Local Includes
#include "renderer.h"
#include "assetmanager.hpp"
#include "texture.h"

//This Include
#include "model.h"

//Implementation
CModel::CModel()
	: m_iMaterialCount(0)
{
	//Constructor
}

CModel::~CModel()
{
	//Destructor
}

IMesh*
CModel::GetMeshObject(unsigned int _uiIndex) const
{
	return(m_vecMeshes[_uiIndex]);
}

unsigned int
CModel::GetMeshCount() const
{
	return((unsigned int)m_vecMeshes.size());
}

TModelMeshInstance
CModel::GetInstance(unsigned int _uiIndex) const
{
	return(m_vecInstances[_uiIndex]);
}

unsigned int
CModel::GetInstanceCount() const
{
	return((unsigned int)m_vecInstances.size());
}

void
CModel::SetMaterial(int _iMatID, const TMaterial& _rtMaterial)
{
	for(unsigned int i = 0; i < m_vecMeshes.size(); ++i)
	{
		if(m_vecMeshes[i]->GetMaterialId() == _iMatID) m_vecMeshes[i]->SetMaterial(_rtMaterial);
	}
}

int
CModel::GetMaterialCount() const
{
	return(m_iMaterialCount);
}

EAssetType
CModel::GetAssetType()
{
	return(ASSET_MODEL);
}

bool
CModel::Load(const char* _strFile)
{
	bool bSuccessful = false;

	//Set up assimp and import the mesh
	Assimp::Importer assetImporter;
	unsigned int uiFlags = (aiProcess_Triangulate |			//Triangulate all quad meshes
							aiProcess_CalcTangentSpace |	//Calc tangent space for t-space normal maps
							aiProcess_GenBoundingBoxes |	//Pre-gen of bounding boxes, saves re-writing this code below
							aiProcess_GlobalScale |			//Fixing the sizing of models
							aiProcess_MakeLeftHanded |		//For DirectX
							aiProcess_FlipWindingOrder |	//Fixes normals for LH system
							aiProcess_OptimizeGraph |		//Collapse empty nodes
							aiProcess_OptimizeMeshes |		//Collapse empty transform nodes (fixes rotation of child objects)
							aiProcess_PopulateArmatureData |//Generates armature info
							aiProcess_FlipUVs);				//A fix for UV's which saves flipping in the shader

	//Read in the scene
	const aiScene* scene = assetImporter.ReadFile(_strFile, uiFlags);
	m_iMaterialCount = scene->mNumMaterials;

	//Mesh Loading
	if(scene->HasMeshes())
	{
		//TODO: load anims
		bool test = scene->HasAnimations();

		if(test)
		{
			//anims contain keyframes for pos/rot/scale indepedently
			//name
			//dur
			//fps (ticks per sec)
			//channels
			//channels are nodes, or bones, such as pelvis/foot etc.
			//channels have node name, keyframes, pre/post state aiAnimBehaviour
			//mesh channels and morph mesh channels I am not sure of, maybe visemes/eyebrows/attachments/sockets etc.
			auto anims = scene->mAnimations;
			bool test2 = false;
		}

		//For each mesh
		for(unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			TVertexTexNorm* pVertices = nullptr;
			DWORD* pIndices = nullptr;
			aiMesh* pSourceMesh = scene->mMeshes[i]; //Get scene mesh

			//Check to see if mesh has vertex data, followed by a check to see if the mesh has indicies and is triangulated
			if(!pSourceMesh->HasPositions())
			{
				continue; //mesh has no vertices?
			}
			else if(pSourceMesh->HasFaces() && pSourceMesh->mFaces[0].mNumIndices != 3)
			{
				continue; //not a triangulated mesh
			}

			//pSourceMesh->mBitangents;
			bool hasbones = pSourceMesh->HasBones();

			//We should leave the tree processing to CArmature, passing in the mesh so that it can extract itself
			//We will need to build the weights here for the mesh though
			if(hasbones)
			{
				//TODO:
				// 
				// BoneID and weights should be stored in a separate buffer
				// bound at IA stage to reduce duplicate code here.
				// 
				// If we try put the weights into its own TVertexTexRigged then we need to make edge
				// cases everywhere for selecting which vertex for zero gain where we can
				// use a second buffer that's independant like the instance buffer.
				// 
				// Each vertex of a rigged mesh will require:
				//  - 4x BoneIDs (used to look up matrix in the bone table such that offsets can be applied)
				//  - 4x Weights (used to calculate weighting between the affectors)
				//
				// that is straight forward, can be done here optimally with CMesh updated for granular control over extra buffers
				// the shader will be the same, a modification from instancing with little effort
				// 
				// On top of that, we need a umap of bones where the key is the bone NAME (string)
				// Where each value is:
				//  - bone id, number for reference to the matrix table which allows for shader lookup
				//  - bone offset, the base offset for the bone, this will be combined with the bone matrix in the animation to produce the final matrix for weighted verts
				//
				// this map will most likely be created and held by CArmature. Technically we don't actually
				// need a bone tree just yet, as the lookup is enough, however in future we will need to produce a tree.
				// CArmature can be linked to a CAnimator using CAnimation classes for all the required information
				//
				//
				//

				//bones link to an armature, explore armature in the skeleton class
				//name
				//numWeights = number of vertices this bone effects
				//armature is the node tree for the bones, not sure why this exists as it's the parent of the following node pointer
				//node pointer which links to the current armature node
				//weights for each vertex
				//offset matrix for the bone
				auto bones = pSourceMesh->mBones;
				bool test2 = false;

				//root node can be found from traversing the parents of any node
				//root node has two children
				//first child seems to be the mesh binding for the armature, it contains the mesh ID and mesh name
				//second child is the pelvis postrotation (is this caused by an import flag)
				//	this node has a unique transformation matrix
				//child of the pelvis_pr is the actual pelvis bone, which contains the identity transform
				//  the next three children of pelvis are spine and thigh PR's

				//looks like the postrot nodes contain the bone transform and
				//	the normal node only contains the bone matching info found in mBones
				//still not sure why they're split up unless the identity transform is for scaling/offset
				//
				//most likely have to flatten the armature to easy traversal
				//check assimp docs to see
			}

			//Vertices
			pVertices = new TVertexTexNorm[pSourceMesh->mNumVertices];

			bool bHasNormal = pSourceMesh->HasNormals();
			bool bHasTangent = pSourceMesh->HasTangentsAndBitangents();
			bool bHasUV = pSourceMesh->HasTextureCoords(0);

			//Debug break for multi-channel UVs, I believe assimp supports up to 8
			if(pSourceMesh->GetNumUVChannels() > 1)
			{
				//currently unsupported
				int uvcount = pSourceMesh->GetNumUVChannels();
				bool breakHereToFix = true;
			}

			for(unsigned int j = 0; j < pSourceMesh->mNumVertices; ++j)
			{
				//TODO: Support multiple UV channels
				pVertices[j].pos = float3(pSourceMesh->mVertices[j].x, pSourceMesh->mVertices[j].y, pSourceMesh->mVertices[j].z);
				if(bHasNormal)	pVertices[j].normal = float3(pSourceMesh->mNormals[j].x, pSourceMesh->mNormals[j].y, pSourceMesh->mNormals[j].z);
				if(bHasTangent) pVertices[j].tangent = float3(pSourceMesh->mTangents[j].x, pSourceMesh->mTangents[j].y, pSourceMesh->mTangents[j].z);
				if(bHasUV)		pVertices[j].texcoord = float2(pSourceMesh->mTextureCoords[0][j].x, pSourceMesh->mTextureCoords[0][j].y);
			}

			//Indices, check if we have faces and that they are triangulated
			if(pSourceMesh->HasFaces() && pSourceMesh->mFaces[0].mNumIndices == 3)
			{
				const unsigned int uiFaceCount = pSourceMesh->mNumFaces;
				const unsigned int uiIndexCount = pSourceMesh->mFaces[0].mNumIndices;
				pIndices = new DWORD[uiFaceCount * uiIndexCount];

				//Could memcpy but UINT to ULONG may cause issues in future
				//Loop through each face, then loop through each index of face (code supports tri/quad even though we only support tri)
				for(unsigned int uiFace = 0; uiFace < uiFaceCount; ++uiFace)
				{
					for(unsigned int uiIndex = 0; uiIndex < uiIndexCount; ++uiIndex)
					{
						pIndices[(uiFace * uiIndexCount) + uiIndex] = pSourceMesh->mFaces[uiFace].mIndices[uiIndex];
					}
				}
			}

			//New mesh data, read only with memory handover
			TMeshData<TVertexTexNorm> tMeshInit(pVertices, pSourceMesh->mNumVertices,
				pIndices, pSourceMesh->mNumFaces * 3,
				EMeshAccess::RAW,
				EMeshAccess::RAW, true);

			//Material
			tMeshInit.iMaterialId = pSourceMesh->mMaterialIndex;

			//Load AABB from the source mesh
			aiAABB tSourceAABB = pSourceMesh->mAABB;
			float3 vec3minPoint = float3(tSourceAABB.mMin.x, tSourceAABB.mMin.y, tSourceAABB.mMin.z);
			float3 vec3maxPoint = float3(tSourceAABB.mMax.x, tSourceAABB.mMax.y, tSourceAABB.mMax.z);
			tMeshInit.vec3BBCenter = (vec3minPoint + vec3maxPoint) * 0.5f;
			tMeshInit.vec3BBExtends = (vec3maxPoint - vec3minPoint) * 0.5f;

			//Create and store new mesh
			CMesh<TVertexTexNorm>* pTargetMesh = new CMesh<TVertexTexNorm>();
			bSuccessful = pTargetMesh->Initialize(CAssetManager::GetInstance().GetRenderer(), tMeshInit);

			//Store
			m_vecMeshes.push_back(pTargetMesh);
		}

		//TODO: Individual models load in fine, but full scenes may be rotated 90 deg...
		//		may have to check metadata or wherever the axis info is
		int iUpAxis, iRightAxis, iLookAxis;
		int iUpSign, iRightSign, iLookSign;

		//Get axis'
		scene->mMetaData->Get("UpAxis", iUpAxis);
		scene->mMetaData->Get("CoordAxis", iRightAxis);
		scene->mMetaData->Get("FrontAxis", iLookAxis);

		//Set sign/value
		scene->mMetaData->Get("UpAxisSign", iUpSign);
		scene->mMetaData->Get("CoordAxisSign", iRightSign);
		scene->mMetaData->Get("FrontAxisSign", iLookSign);

		//Up, Right, Fwd
		float3 vec3Orientation[3];
		vec3Orientation[0][iUpAxis] = iUpSign;
		vec3Orientation[1][iRightAxis] = iRightSign;
		vec3Orientation[2][iLookAxis] = iLookSign;
		
		//TODO: Cover all edge cases here for different co-ord systems
		float3 vec3RootTranform[3];
		//(x = 0, y = 1, z = 2)
		//Rotate pitch by 90 deg if up is on the Z axis (iUpSign will flip -/+)
		vec3RootTranform[2].x = (iUpAxis == 2 ? XMConvertToRadians(90.0f * iUpSign) : 0.0f);

		ProcessSceneNodes(scene->mRootNode, vec3Orientation, vec3RootTranform);
	}

	//Release scene
	assetImporter.FreeScene();

	//TODO: Double check all cases here
	return(bSuccessful);
}

void
CModel::Release()
{
	//Destructor
	for(unsigned int i = 0; i < m_vecMeshes.size(); ++i)
	{
		if(!m_vecMeshes[i]) continue;
		delete m_vecMeshes[i];
		m_vecMeshes[i] = nullptr;
	}

	m_vecInstances.clear(); //Only references so clear this
	m_vecMeshes.clear();
}

void
CModel::ProcessSceneNodes(aiNode* _pNode, const float3* _vec3UpRightFwd, float3 _vec3PosScaleRot[3])
{
	//TODO: Revise this whole function to account for the oddities of orientation as well as memory consumption
	//		This could be optimized and fixed a lot
	

	//Decompose node's transform
	aiVector3D aivec3Pos, aivec3Rot, aivec3Scale;
	_pNode->mTransformation.Decompose(aivec3Scale, aivec3Rot, aivec3Pos);

	//flip position based on orientation
	//if Y is not the up-axis, assume Z is up
	//TODO: add edge-cases here where Z is not up
	if(!_vec3UpRightFwd[0].y) aivec3Pos = {aivec3Pos.x, -aivec3Pos.z, aivec3Pos.y};

	//TODO: Fix this, check meta data, consider the following issue below too
	// https://github.com/assimp/assimp/issues/849
	// I've used absolute scales due to some scenes using negative values to "flip" a mesh
	//		which obviously gives inverted normals. It's a pain to fix with instanced meshes
	//		where you'd need to add a flag, or render on both sides, unless
	//		there is a postprocessing flag which makes a copy of the mesh with correct norms?
	//
	//TODO: From this, maybe enable flipped normals if size is negative
	float3 vec3Transform[3];
	vec3Transform[0] = _vec3PosScaleRot[0] + float3(aivec3Pos.x, aivec3Pos.y, aivec3Pos.z);
	vec3Transform[1] = float3(fabsf(aivec3Scale.x), fabsf(aivec3Scale.y), fabsf(aivec3Scale.z));
	vec3Transform[2] = _vec3PosScaleRot[2] + float3(aivec3Rot.x, aivec3Rot.y, aivec3Rot.z);

	//If this node has meshes
	if(_pNode->mNumMeshes > 0)
	{
		//Build instance data
		TModelMeshInstance tInstance;
		tInstance.vec3Pos = vec3Transform[0];
		tInstance.vec3Scale = vec3Transform[1];
		tInstance.vec3Rot = float3(XMConvertToDegrees(vec3Transform[2].x), XMConvertToDegrees(vec3Transform[2].y), XMConvertToDegrees(vec3Transform[2].z));

		//For each mesh in this node
		for(unsigned int i = 0; i < _pNode->mNumMeshes; ++i)
		{
			//Build instance data
			tInstance.uiMeshID = _pNode->mMeshes[i]; //Link to mesh ID

			//Store to instance
			m_vecInstances.push_back(tInstance);
		}
	}

	//Explore the rest of the scene
	for(unsigned int i = 0; i < _pNode->mNumChildren; ++i) ProcessSceneNodes(_pNode->mChildren[i], _vec3UpRightFwd, vec3Transform);
}
