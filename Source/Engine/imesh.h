#pragma once
#ifndef __IMESH_H__
#define __IMESH_H__

//Enums
enum class EMeshAccess
{
	INVALID_ACCESS = -1,

	RAW,		//Vertices/Indices passed in will be stored in a buffer for draw only, no read/write
	RAW_WRITE,	//Writing must be done in full for the draw range as buffer does not store old data
	READ,		//Vertices/Indices passed in will be stored locally for reading, buffer will remain static (no-write)
	READWRITE,	//Vertices/Indices passed in will be stored locally and copied to the buffer when edited
};

//Structures
template <typename TVertexType, typename TIndexType = DWORD>
struct TMeshData
{
	//Variables
	//Only requiring data for creation, nothing more.
	TVertexType* pVertices;
	TIndexType* pIndices;

	UINT uiVertexCount;
	UINT uiIndexCount;
	//UINT uiVertexSize; //Assumed via template of CMesh
	//UINT uiIndexSize; //Assumed via template of CMesh

	EMeshAccess eVBufferAccess;
	EMeshAccess eIBufferAccess;

	D3D11_PRIMITIVE_TOPOLOGY tVertexTopology;

	//TODO: Fix this with either std::unique_ptr etc.
	bool bPointerOwnership; //HACK: Take ownership of vertex/index buffer during init. (Hack for CModel)

	//TODO: Fix constructor functions to allow these?
	//Bounding Box information
	float3 vec3BBCenter;
	float3 vec3BBExtends; //Generate sphere from max x/y/z

	//TODO: Add in ID/name?
	//const char* pcName[64]; //64 is quite long, consider 32 (the quick brown fox jumped over)
	int iMaterialId;

	//Functions
	//Empty Struct
	TMeshData()
		: pVertices(nullptr)
		, pIndices(nullptr)
		, uiVertexCount(0)
		, uiIndexCount(0)
		, eVBufferAccess(EMeshAccess::RAW)
		, eIBufferAccess(EMeshAccess::RAW)
		, tVertexTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		, bPointerOwnership(false)
		, vec3BBCenter(0.0f, 0.0f, 0.0f)
		, vec3BBExtends(0.0f, 0.0f, 0.0f)
		, iMaterialId(-1)
	{
		//Constructor
	}

	//Vertex Buffer Only
	TMeshData(TVertexType* _pVertices, UINT _uiVertexCount, EMeshAccess _eVBufferAccess = EMeshAccess::RAW, bool _bPointerOwnership = false)
		: pVertices(_pVertices)
		, pIndices(nullptr)
		, uiVertexCount(_uiVertexCount)
		, uiIndexCount(0)
		, eVBufferAccess(_eVBufferAccess)
		, eIBufferAccess(EMeshAccess::RAW)
		, tVertexTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		, bPointerOwnership(_bPointerOwnership)
		, vec3BBCenter(0.0f, 0.0f, 0.0f)
		, vec3BBExtends(0.0f, 0.0f, 0.0f)
		, iMaterialId(-1)
	{
		//Constructor
	}

	//Vertex and Index buffer
	TMeshData(TVertexType* _pVertices, UINT _uiVertexCount, TIndexType* _pIndices, UINT _uiIndexCount, EMeshAccess _eVBufferAccess = EMeshAccess::RAW, EMeshAccess _eIBufferAccess = EMeshAccess::RAW, bool _bPointerOwnership = false)
		: pVertices(_pVertices)
		, pIndices(_pIndices)
		, uiVertexCount(_uiVertexCount)
		, uiIndexCount(_uiIndexCount)
		, eVBufferAccess(_eVBufferAccess)
		, eIBufferAccess(_eIBufferAccess)
		, tVertexTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		, bPointerOwnership(_bPointerOwnership)
		, vec3BBCenter(0.0f, 0.0f, 0.0f)
		, vec3BBExtends(0.0f, 0.0f, 0.0f)
		, iMaterialId(-1)
	{
		//Constructor
	}
};

//Prototypes
class IShader;
struct TMaterial;
class IInstancePool;
template<typename T> struct TRange;
class IMesh
{
	//Member Functions
protected:
	~IMesh() = default;

public:
	virtual bool Draw(float4x4* _pmatWorld, IShader* _pShader = nullptr) = 0;
	virtual bool DrawInstanced(IInstancePool* _pInstancePool, TRange<unsigned int> _tInstanceRange, IShader* _pShader = nullptr) = 0;

	virtual void BindToIA(IInstancePool* _pInstancePool = nullptr) = 0;

	virtual void SetMaterial(const TMaterial& _rtMaterial) = 0;
	virtual TMaterial GetMaterial() const = 0;
	virtual int GetMaterialId() const = 0;

	virtual bool SetVertexRange(unsigned int _uiStart, unsigned int _uiLength = -1) = 0;
	virtual bool SetIndexRange(unsigned int _uiStart, unsigned int _uiLength = -1) = 0;
	virtual const TRange<unsigned int>& GetVertexRange() const = 0;
	virtual const TRange<unsigned int>& GetIndexRange() const = 0;

	//Gets number of Vertices/Indices
	virtual unsigned int GetVertexCount() const = 0;
	virtual unsigned int GetIndexCount() const = 0;

	//Get size of types
	virtual size_t GetVertexSize() const = 0;
	virtual size_t GetIndexSize() const = 0;

	//AABB and Bounding Sphere get functions
	virtual const DirectX::BoundingBox& GetBoundingBox() const = 0;
	virtual const DirectX::BoundingSphere& GetBoundingSphere() const = 0;

};

#endif //__IMESH_H__
