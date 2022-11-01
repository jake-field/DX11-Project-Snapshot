#pragma once
#ifndef __MESH_H__
#define __MESH_H__

//Library Includes
#include <DirectXCollision.h> //Included for Bounding Box/Sphere
#include <vector>

//Local Includes
#include "common.h"
#include "dxcommon.h"
#include "imesh.h"
#include "iinstancepool.h"
#include "renderer.h"
#include "ishader.h"
#include "material.h"

//Template assistance, used for the implementation to allow for quick changing of template specifics
#define CMESH_TEMPLATE template<typename TVertexType, typename TIndexType>
#define CMESH_INSERT TVertexType, TIndexType

//Prototypes
template <typename TVertexType, typename TIndexType = DWORD>
class CMesh final: public IMesh
{
	//Member Functions
public:
	CMesh();
	~CMesh();

	//Init and create the buffer
	bool Initialize(CRenderer* _pRenderer, const TMeshData<CMESH_INSERT>& _rtMeshData);

	//Draw functions
	bool Draw(float4x4* _pmatWorld, IShader* _pShader = nullptr);
	bool DrawInstanced(IInstancePool* _pInstancePool, TRange<unsigned int> _tInstanceRange, IShader* _pShader = nullptr);

	//Binds the mesh to the Input Assembler Stage in prep for drawing
	void BindToIA(IInstancePool* _pInstancePool = nullptr);

	//Assign texture to mesh primitive
	void SetMaterial(const TMaterial& _rtMaterial);
	TMaterial GetMaterial() const;
	int GetMaterialId() const;

	//TODO: BoundingBox/Sphere Gen
	const DirectX::BoundingBox& GetBoundingBox() const;
	const DirectX::BoundingSphere& GetBoundingSphere() const;

	//Update topology
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY _eTopology);
	D3D11_PRIMITIVE_TOPOLOGY GetTopology() const;

	//Draw range adjustments, allows select rendering of the vbuffer/ibuffer
	//	_iStart is an index, not bytes
	//	if _iLength is -1, run to end of buffer
	//	Returns false if input produced out of range result
	bool SetVertexRange(unsigned int _uiStart, unsigned int _uiLength = -1);
	bool SetIndexRange(unsigned int _uiStart, unsigned int _uiLength = -1);
	const TRange<unsigned int>& GetVertexRange() const;
	const TRange<unsigned int>& GetIndexRange() const;

	//Gets number of Vertices/Indices
	unsigned int GetVertexCount() const;
	unsigned int GetIndexCount() const;

	//Get size of types
	size_t GetVertexSize() const;
	size_t GetIndexSize() const;

	//Read functions, returns NULL if not a readable mesh, or out of bounds
	const TVertexType* GetVertex(unsigned int _uiVertex) const; //Returns single TVertexType
	const TIndexType* GetIndex(unsigned int _uiIndex) const; //Returns single TIndexType

	//Write functions, returns FALSE if not a writeable mesh, or out of bounds
	bool SetVertex(unsigned int _uiVertex, const TVertexType& _pNewData);
	bool SetVertices(unsigned int _uiStart, unsigned int _uiLength, const TVertexType* _pNewData);
	bool SetIndex(unsigned int _uiIndex, const TIndexType& _pNewData);
	bool SetIndices(unsigned int _uiStart, unsigned int _uiLength, const TIndexType* _pNewData);

	//Read/Write checks
	bool CanReadVB() const;
	bool CanReadIB() const;
	bool CanWriteVB() const;
	bool CanWriteIB() const;

protected:
	bool InternalDraw(IInstancePool* _pInstancePool, TRange<unsigned int> _tInstanceRange, float4x4* _pmatWorld, IShader* _pShader);
	bool OpenBuffers(bool _bVBuffer, bool _bIBuffer = false);
	bool CopyBuffers(bool _bVBuffer, bool _bIBuffer = false);
	void CloseBuffers(bool _bVBuffer = true, bool _bIBuffer = true);

	//Member Variables
protected:
	//Renderer
	CRenderer* m_pRenderer;

	//Primitive used for buffers and rendering
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	TRange<unsigned int> m_tVertexRange;
	TRange<unsigned int> m_tIndexRange;

	//Texture storage
	TMaterial m_tMaterial;
	int m_iMaterialId;

	//BB and Bounding Sphere
	DirectX::BoundingBox m_tBoundingBox;
	DirectX::BoundingSphere m_tBoundingSphere;

	//Readable Mesh Data
	TMeshData<CMESH_INSERT> m_tMesh;

	//State for supporting write access
	D3D11_MAPPED_SUBRESOURCE m_pMappedVBuffer;
	D3D11_MAPPED_SUBRESOURCE m_pMappedIBuffer;
	bool m_bUpdateVBuffer;
	bool m_bUpdateIBuffer;

};


//------------------------------------------------------------------------
//Implementation
//------------------------------------------------------------------------
CMESH_TEMPLATE
CMesh<CMESH_INSERT>::CMesh()
	: m_pRenderer(nullptr)
	, m_pVertexBuffer(nullptr)
	, m_pIndexBuffer(nullptr)
	, m_tVertexRange(0, 0)
	, m_tIndexRange(0, 0)
	, m_iMaterialId(-1)
	, m_bUpdateVBuffer(false)
	, m_bUpdateIBuffer(false)
{
	//Constructor
	ZeroMemory(&m_tMaterial, sizeof(TMaterial));
	ZeroMemory(&m_tMesh, sizeof(TMeshData<CMESH_INSERT>));
	ZeroMemory(&m_pMappedVBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&m_pMappedIBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&m_tBoundingBox, sizeof(DirectX::BoundingBox));
	ZeroMemory(&m_tBoundingSphere, sizeof(DirectX::BoundingSphere));
}

CMESH_TEMPLATE
CMesh<CMESH_INSERT>::~CMesh()
{
	//Destructor
	CloseBuffers(); //Close the buffers if they were open

	m_pRenderer = nullptr;

	//Release buffers
	ReleaseCOM(m_pVertexBuffer);
	ReleaseCOM(m_pIndexBuffer);

	//Textures are handled by the Asset Manager, do not release or call delete[]
	ZeroMemory(&m_tMaterial, sizeof(TMaterial));

	//Delete mesh data if any and zero the structure
	SafeDeleteArray(m_tMesh.pVertices);
	SafeDeleteArray(m_tMesh.pIndices);
	ZeroMemory(&m_tMesh, sizeof(TMeshData<CMESH_INSERT>));
	ZeroMemory(&m_pMappedVBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&m_pMappedIBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&m_tBoundingBox, sizeof(DirectX::BoundingBox));
	ZeroMemory(&m_tBoundingSphere, sizeof(DirectX::BoundingSphere));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::Initialize(CRenderer* _pRenderer, const TMeshData<CMESH_INSERT>& _rtMeshData)
{
	//Return value
	bool bSuccessful = false;

	//Set renderer
	m_pRenderer = _pRenderer;

	//Set up mesh information
	m_tMesh.uiVertexCount = _rtMeshData.uiVertexCount;
	m_tMesh.uiIndexCount = _rtMeshData.uiIndexCount;
	m_tMesh.eVBufferAccess = _rtMeshData.eVBufferAccess;
	m_tMesh.eIBufferAccess = _rtMeshData.eIBufferAccess;
	m_tMesh.tVertexTopology = _rtMeshData.tVertexTopology;
	m_tVertexRange = {0, m_tMesh.uiVertexCount};
	m_tIndexRange = {0, m_tMesh.uiIndexCount};
	m_iMaterialId = _rtMeshData.iMaterialId;

	//Set up bounding box and bounding sphere
	m_tBoundingBox.Center = _rtMeshData.vec3BBCenter;
	m_tBoundingBox.Extents = _rtMeshData.vec3BBExtends;
	DirectX::BoundingSphere::CreateFromBoundingBox(m_tBoundingSphere, m_tBoundingBox);

	//Buffer usage type, if mesh is writable, it needs to be a dynamic buffer
	//TODO: Support staging and immutable?
	D3D11_USAGE eVBufferUsage = CanWriteVB() ? D3D11_USAGE::D3D11_USAGE_DYNAMIC : D3D11_USAGE::D3D11_USAGE_DEFAULT;
	D3D11_USAGE eIBufferUsage = CanWriteIB() ? D3D11_USAGE::D3D11_USAGE_DYNAMIC : D3D11_USAGE::D3D11_USAGE_DEFAULT;

	//Create the VBuffer, then IBuffer if that succeeds
	//If readable, create the local mesh data as well
	//-------------------------------------------------
	//If there are vertices and the vertex count is valid
	if((CanWriteVB() || _rtMeshData.pVertices) && _rtMeshData.uiVertexCount > 0)
	{
		//If readable, create a local copy of the vertices
		if(CanReadVB() && !_rtMeshData.bPointerOwnership)
		{
			//Create local vertex array
			m_tMesh.pVertices = new TVertexType[_rtMeshData.uiVertexCount];

			if(_rtMeshData.pVertices && m_tMesh.pVertices)
			{
				//TODO: Check memcpy_s return, not that it matters much here but there is a chance of failure
				//Copy vertex data if it's valid (as it may not be if CanWriteVB() == true)
				memcpy_s(m_tMesh.pVertices, sizeof(TVertexType) * m_tMesh.uiVertexCount, _rtMeshData.pVertices, sizeof(TVertexType) * _rtMeshData.uiVertexCount);
			}
		}
		else if(_rtMeshData.bPointerOwnership) //Ignore CanReadVB() as we check for it below if we need to delete the data
		{
			//Being passed vertex data
			m_tMesh.pVertices = _rtMeshData.pVertices;
		}

		//Request GPU Lock as creating a buffer needs exclusive gpu access
		m_pRenderer->GetGPUMutex().lock();

		//Create vertex buffer
		unsigned int uiVBufferSize = _rtMeshData.uiVertexCount * sizeof(TVertexType);
		m_pVertexBuffer = m_pRenderer->CreateBuffer(D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, _rtMeshData.pVertices, uiVBufferSize, eVBufferUsage);

		//Release GPU Lock here as other threads may need it while we continue logic here
		m_pRenderer->GetGPUMutex().unlock();

		//Clean-up vertex array if we own it and don't need to read it, as we are now done with the vertex buffer
		if(!CanReadVB() && _rtMeshData.bPointerOwnership) SafeDelete(m_tMesh.pVertices);

		//Check if the vertex buffer was created, otherwise don't both creating index buffer
		if(m_pVertexBuffer)
		{
			//If there are indices and the index count is valid
			if((CanWriteIB() || _rtMeshData.pIndices) && _rtMeshData.uiIndexCount > 0)
			{
				//If readable, create a local copy of the indices
				if(CanReadIB() && !_rtMeshData.bPointerOwnership)
				{
					//Create index array
					m_tMesh.pIndices = new TIndexType[_rtMeshData.uiIndexCount];

					if(_rtMeshData.pIndices && m_tMesh.pIndices)
					{
						//TODO: Check memcpy_s return, not that it matters much here but there is a chance of failure
						//Copy index data if it's valid (as it may not be if CanWriteIB() == true)
						memcpy_s(m_tMesh.pIndices, sizeof(TIndexType) * m_tMesh.uiIndexCount, _rtMeshData.pIndices, sizeof(TIndexType) * _rtMeshData.uiIndexCount);
					}
				}
				else if(_rtMeshData.bPointerOwnership) //Ignore CanReadIB() as we check for it below if we need to delete the data
				{
					//Being passed index data
					m_tMesh.pIndices = _rtMeshData.pIndices;
				}

				//Request GPU Lock as creating a buffer needs exclusive gpu access
				m_pRenderer->GetGPUMutex().lock();

				//Create index buffer
				unsigned int uiIBufferSize = _rtMeshData.uiIndexCount * sizeof(TIndexType);
				m_pIndexBuffer = m_pRenderer->CreateBuffer(D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER, _rtMeshData.pIndices, uiIBufferSize, eIBufferUsage);

				//Release GPU Lock
				m_pRenderer->GetGPUMutex().unlock();

				//Clean-up the index array if we own it and don't need to read it, as we are now done with the index buffer
				if(!CanReadIB() && _rtMeshData.bPointerOwnership) SafeDelete(m_tMesh.pIndices);

				//Success if index buffer was created (we only get here if vbuffer was created too)
				bSuccessful = (m_pIndexBuffer != nullptr);
			}
			else
			{
				//No indices, not important
				//Vbuffer was made if we got here
				bSuccessful = true;
			}
		}
	}

	//Zeroing if fail
	if(!bSuccessful)
	{
		ZeroMemory(&m_tMesh, sizeof(TMeshData<CMESH_INSERT>));
		ReleaseCOM(m_pVertexBuffer);
		ReleaseCOM(m_pIndexBuffer);
	}

	return(bSuccessful);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::Draw(float4x4* _pmatWorld, IShader* _pShader)
{
	//Non-instanced draw call, ignoring the instancer and instance range
	return(InternalDraw(nullptr, {0, 0}, _pmatWorld, _pShader));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::DrawInstanced(IInstancePool* _pInstancePool, TRange<unsigned int> _tInstanceRange, IShader* _pShader)
{
	//Call to draw ignoring matWorld
	return(InternalDraw(_pInstancePool, _tInstanceRange, nullptr, _pShader));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::InternalDraw(IInstancePool* _pInstancePool, TRange<unsigned int> _tInstanceRange, float4x4* _pmatWorld, IShader* _pShader)
{
	//This function is convoluted because separating both to Draw/DrawInstanced just duplicates code for no valid reason
	bool bSuccessful = false;
	IShader* pShader = !_pShader ? IShader::GetActiveShader() : _pShader;

	//Check if we should update the buffers
	//	Since Draw(); is main thread blocking, we don't need to get the GPU lock before editing
	if(m_bUpdateVBuffer || m_bUpdateIBuffer) CopyBuffers(m_bUpdateVBuffer, m_bUpdateIBuffer);

	//Close the buffers prior to drawing just in case they have been opened
	//	CopyBuffers does not close the buffers in case of minor/continuous edits.
	//	Calling CloseBuffers() has minimal perf impact as it silently fails if buffers are already closed.
	CloseBuffers();

	//Pointer checks for renderer and the incoming params
	if(m_pRenderer && m_pRenderer->GetDeviceContext() && pShader)
	{
		//Prep the shader for drawing us
		if(pShader->Predraw(this, _pmatWorld, (_pInstancePool != nullptr)))
		{
			//Bind our mesh to the Input assembler stage
			BindToIA(_pInstancePool);

			//Select appropriate draw function
			if(m_pIndexBuffer)
			{
				if(!_pInstancePool) m_pRenderer->GetDeviceContext()->DrawIndexed(m_tIndexRange.b, m_tIndexRange.a, 0);
				else m_pRenderer->GetDeviceContext()->DrawIndexedInstanced(m_tIndexRange.b, _tInstanceRange.b, m_tIndexRange.a, 0, _tInstanceRange.a);
			}
			else
			{
				if(!_pInstancePool) m_pRenderer->GetDeviceContext()->Draw(m_tVertexRange.b, m_tVertexRange.a);
				else m_pRenderer->GetDeviceContext()->DrawInstanced(m_tVertexRange.b, _tInstanceRange.b, m_tVertexRange.a, _tInstanceRange.a);
			}

			bSuccessful = true; //Assume success as there are no checks past here to determine that
		}
	}

	//Draw via the shader
	return(bSuccessful);
}

CMESH_TEMPLATE
void CMesh<CMESH_INSERT>::BindToIA(IInstancePool* _pInstancePool)
{
	//Verify renderer and context are available
	if(m_pRenderer && m_pRenderer->GetDeviceContext())
	{
		DXGI_FORMAT eIndexFormat = DXGI_FORMAT_R32_UINT; //TODO: Match this to TIndexType
		
		//_pMeshInstancer may be null, but having [2] doesn't harm anything performance/memory wise, so this works fine
		unsigned int uiStrides[2] = {sizeof(TVertexType), _pInstancePool ? _pInstancePool->GetStride() : 0};
		unsigned int uiOffsets[2] = {0, 0}; //We don't support offsets, this will be done in draw calls
		ID3D11Buffer* const pBuffers[2] = {m_pVertexBuffer, _pInstancePool ? _pInstancePool->GetBuffer() : nullptr};

		//Bind this mesh to the IA Stage for rendering. Only need to set if we use it during draw, set buffers are ignored if not drawn
		m_pRenderer->GetDeviceContext()->IASetPrimitiveTopology(m_tMesh.tVertexTopology);
		if(m_pVertexBuffer) m_pRenderer->GetDeviceContext()->IASetVertexBuffers(0, _pInstancePool ? 2 : 1, pBuffers, uiStrides, uiOffsets);
		if(m_pIndexBuffer) m_pRenderer->GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, eIndexFormat, 0);
	}
}

CMESH_TEMPLATE
void CMesh<CMESH_INSERT>::SetMaterial(const TMaterial& _rtMaterial)
{
	m_tMaterial = _rtMaterial;
}

CMESH_TEMPLATE
TMaterial CMesh<CMESH_INSERT>::GetMaterial() const
{
	return(m_tMaterial);
}

CMESH_TEMPLATE
int CMesh<CMESH_INSERT>::GetMaterialId() const
{
	return(m_iMaterialId);
}

CMESH_TEMPLATE
const DirectX::BoundingBox& CMesh<CMESH_INSERT>::GetBoundingBox() const
{
	return(m_tBoundingBox);
}

CMESH_TEMPLATE
const DirectX::BoundingSphere& CMesh<CMESH_INSERT>::GetBoundingSphere() const
{
	return(m_tBoundingSphere);
}

CMESH_TEMPLATE
void CMesh<CMESH_INSERT>::SetTopology(D3D11_PRIMITIVE_TOPOLOGY _eTopology)
{
	m_tMesh.tVertexTopology = _eTopology;
}

CMESH_TEMPLATE
D3D11_PRIMITIVE_TOPOLOGY CMesh<CMESH_INSERT>::GetTopology() const
{
	return(m_tMesh.tVertexTopology);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::SetVertexRange(unsigned int _uiStart, unsigned int _uiLength)
{
	//If length was -1, use vertex count to set full range (include offset)
	if(_uiLength == (unsigned int)(-1)) _uiLength = m_tMesh.uiVertexCount - _uiStart;

	//Check if parameters are in range of the buffer
	bool bInRange = (_uiLength > 0 && (_uiStart + _uiLength) <= m_tMesh.uiVertexCount);

	if(bInRange)
	{
		m_tVertexRange.a = _uiStart;
		m_tVertexRange.b = _uiLength;
	}

	return(bInRange);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::SetIndexRange(unsigned int _uiStart, unsigned int _uiLength)
{
	//If length was -1, use index count to set full range (include offset)
	if(_uiLength == (unsigned int)(-1)) _uiLength = m_tMesh.uiIndexCount - _uiStart;

	//Check if parameters are in range of the buffer
	bool bInRange = (_uiStart + _uiLength) <= m_tMesh.uiIndexCount;

	if(bInRange)
	{
		m_tIndexRange.a = _uiStart;
		m_tIndexRange.b = _uiLength;
	}

	return(bInRange);
}

CMESH_TEMPLATE
const TRange<unsigned int>& CMesh<CMESH_INSERT>::GetVertexRange() const
{
	return(m_tVertexRange);
}

CMESH_TEMPLATE
const TRange<unsigned int>& CMesh<CMESH_INSERT>::GetIndexRange() const
{
	return(m_tIndexRange);
}

CMESH_TEMPLATE
unsigned int CMesh<CMESH_INSERT>::GetVertexCount() const
{
	return(m_tMesh.uiVertexCount);
}

CMESH_TEMPLATE
unsigned int CMesh<CMESH_INSERT>::GetIndexCount() const
{
	return(m_tMesh.uiIndexCount);
}

CMESH_TEMPLATE
size_t CMesh<CMESH_INSERT>::GetVertexSize() const
{
	return(sizeof(TVertexType));
}

CMESH_TEMPLATE
size_t CMesh<CMESH_INSERT>::GetIndexSize() const
{
	return(sizeof(TIndexType));
}

CMESH_TEMPLATE
const TVertexType* CMesh<CMESH_INSERT>::GetVertex(unsigned int _uiVertex) const
{
	TVertexType* pVertex = nullptr;
	if(CanReadVB() && m_tMesh.pVertices && _uiVertex < m_tMesh.uiVertexCount) pVertex = &m_tMesh.pVertices[_uiVertex];
	return(pVertex);
}

CMESH_TEMPLATE
const TIndexType* CMesh<CMESH_INSERT>::GetIndex(unsigned int _uiIndex) const
{
	TIndexType* pIndex = nullptr;
	if(CanReadIB() && m_tMesh.pIndices && _uiIndex < m_tMesh.uiIndexCount) pIndex = &m_tMesh.pIndices[_uiIndex];
	return(pIndex);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::SetVertex(unsigned int _uiVertex, const TVertexType& _pNewData)
{
	bool bValidIndex = CanWriteVB() && _uiVertex < m_tMesh.uiVertexCount;

	if(bValidIndex && m_tMesh.pVertices != nullptr)
	{
		//Write to local storage before updating
		m_tMesh.pVertices[_uiVertex] = _pNewData;
		m_bUpdateVBuffer = true;
	}
	else if(bValidIndex && OpenBuffers(true, false))
	{
		//Write directly to buffer
		reinterpret_cast<TVertexType*>(m_pMappedVBuffer.pData)[_uiVertex] = _pNewData;
	}

	return(bValidIndex);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::SetVertices(unsigned int _uiStart, unsigned int _uiLength, const TVertexType* _pNewData)
{
	//Check if parameters are in range of the buffer
	//	Could adjust uiLength here to bind to uiVertexCount if uiLength == -1, but since
	//	we're manipulating data it's best the caller know exactly what they are changing
	bool bInRange = CanWriteVB() && (_uiStart + _uiLength) < m_tMesh.uiVertexCount;

	if(bInRange && m_tMesh.pVertices != nullptr)
	{
		//Write to local storage before updating
		memcpy_s(&m_tMesh.pVertices[_uiStart], (_uiLength * sizeof(TVertexType)), _pNewData, (_uiLength * sizeof(TVertexType)));
		m_bUpdateVBuffer = true;
	}
	else if(bInRange && OpenBuffers(true, false))
	{
		//Write directly to buffer
		memcpy_s(&m_pMappedVBuffer.pData[_uiStart], (_uiLength * sizeof(TVertexType)), _pNewData, (_uiLength * sizeof(TVertexType)));
	}

	return(bInRange);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::SetIndex(unsigned int _uiIndex, const TIndexType& _pNewData)
{
	bool bValidIndex = CanWriteIB() && _uiIndex < m_tMesh.uiIndexCount;

	if(bValidIndex && m_tMesh.pIndices != nullptr)
	{
		//Write to local storage before updating
		m_tMesh.pIndices[_uiIndex] = _pNewData;
		m_bUpdateIBuffer = true;
	}
	else if(bValidIndex && OpenBuffers(false, true))
	{
		//Write directly to buffer
		reinterpret_cast<TIndexType*>(m_pMappedIBuffer.pData)[_uiIndex] = _pNewData;
	}

	return(bValidIndex);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::SetIndices(unsigned int _uiStart, unsigned int _uiLength, const TIndexType* _pNewData)
{
	//Check if parameters are in range of the buffer
	//	Could adjust uiLength here to bind to uiIndexCount if uiLength == -1, but since
	//	we're manipulating data it's best the caller know exactly what they are changing
	bool bInRange = CanWriteIB() && (_uiStart + _uiLength) <= m_tMesh.uiIndexCount;

	if(bInRange && m_tMesh.pIndices != nullptr)
	{
		memcpy_s(&m_tMesh.pIndices[_uiStart], (_uiLength * sizeof(TIndexType)), _pNewData, (_uiLength * sizeof(TIndexType)));
		m_bUpdateIBuffer = true;
	}
	else if(bInRange && OpenBuffers(false, true))
	{
		//Write directly to buffer
		memcpy_s(&m_pMappedIBuffer.pData[_uiStart], (_uiLength * sizeof(TIndexType)), _pNewData, (_uiLength * sizeof(TIndexType)));
	}

	return(bInRange);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::CanReadVB() const
{
	return(m_pVertexBuffer && (m_tMesh.eVBufferAccess == EMeshAccess::READ || m_tMesh.eVBufferAccess == EMeshAccess::READWRITE));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::CanReadIB() const
{
	return(m_pIndexBuffer && (m_tMesh.eIBufferAccess == EMeshAccess::READ || m_tMesh.eIBufferAccess == EMeshAccess::READWRITE));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::CanWriteVB() const
{
	return(m_pVertexBuffer && (m_tMesh.eVBufferAccess == EMeshAccess::RAW_WRITE || m_tMesh.eVBufferAccess == EMeshAccess::READWRITE));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::CanWriteIB() const
{
	return(m_pIndexBuffer && (m_tMesh.eIBufferAccess == EMeshAccess::RAW_WRITE || m_tMesh.eIBufferAccess == EMeshAccess::READWRITE));
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::OpenBuffers(bool _bVBuffer, bool _bIBuffer)
{
	//Verify renderer and context are available
	if(m_pRenderer && m_pRenderer->GetDeviceContext())
	{
		//If _bVBuffer, check to see if we can write and that the buffer isn't already open
		if(_bVBuffer && CanWriteVB() && !m_pMappedVBuffer.pData)
		{
			m_pRenderer->GetDeviceContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_pMappedVBuffer);
		}

		//If _bIBuffer, check to see if we can write and that the buffer isn't already open
		if(_bIBuffer && CanWriteIB() && !m_pMappedIBuffer.pData)
		{
			m_pRenderer->GetDeviceContext()->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_pMappedIBuffer);
		}
	}

	//True if selected buffer(s) are mapped/opened, false if one or both failed or never set
	bool bBuffersOpen = (_bVBuffer || _bIBuffer) && (_bVBuffer && m_pMappedVBuffer.pData || !_bVBuffer) && (_bIBuffer && m_pMappedIBuffer.pData || !_bIBuffer);

	//Returns true if either buffer successfully updated if requested, false if either failed or none changed
	return(bBuffersOpen);
}

CMESH_TEMPLATE
bool CMesh<CMESH_INSERT>::CopyBuffers(bool _bVBuffer, bool _bIBuffer)
{
	bool bVBufferSet = false;
	bool bIBufferSet = false;

	//Make sure that the requested buffers for copy have the correct config
	bool bWrite = (_bVBuffer && CanWriteVB()) || (_bIBuffer && CanWriteIB());

	//Attempt to open the buffer(s) if the config is correct
	if(bWrite && OpenBuffers(_bVBuffer, _bIBuffer))
	{
		//Update the vertex buffer if it was mapped
		if(_bVBuffer && m_pMappedVBuffer.pData)
		{
			bVBufferSet = !memcpy_s(m_pMappedVBuffer.pData, (m_tMesh.uiVertexCount * sizeof(TVertexType)), m_tMesh.pVertices, (m_tMesh.uiVertexCount * sizeof(TVertexType)));
			m_bUpdateVBuffer = m_bUpdateVBuffer ? !bVBufferSet : false; //Ensure data is only updated once successfully. Flag for update if update failed
		}

		//Update the index buffer if it was mapped
		if(_bIBuffer && m_pMappedIBuffer.pData)
		{
			bIBufferSet = !memcpy_s(m_pMappedIBuffer.pData, (m_tMesh.uiIndexCount * sizeof(TIndexType)), m_tMesh.pIndices, (m_tMesh.uiIndexCount * sizeof(TIndexType)));
			m_bUpdateIBuffer = m_bUpdateIBuffer ? !bIBufferSet : false; //Ensure data is only updated once successfully. Flag for update if update failed
		}
	}

	//True if selected buffer(s) are updated, false if one or both failed
	bool bBuffersCopied = bWrite && (_bVBuffer && bVBufferSet || !_bVBuffer) && (_bIBuffer && bIBufferSet || !_bIBuffer);

	//Returns true if either buffer successfully updated if requested, false if failed or no changes made
	return(bBuffersCopied);
}

CMESH_TEMPLATE
void CMesh<CMESH_INSERT>::CloseBuffers(bool _bVBuffer, bool _bIBuffer)
{
	//Verify renderer and context are available
	if(m_pRenderer && m_pRenderer->GetDeviceContext())
	{
		//If buffer is open/mapped, close/unmap it
		if(_bVBuffer && m_pMappedVBuffer.pData)
		{
			m_pRenderer->GetDeviceContext()->Unmap(m_pVertexBuffer, 0);
			m_pMappedVBuffer.pData = nullptr; //unset to prevent issues
		}

		//If buffer is open/mapped, close/unmap it
		if(_bIBuffer && m_pMappedIBuffer.pData)
		{
			m_pRenderer->GetDeviceContext()->Unmap(m_pIndexBuffer, 0);
			m_pMappedIBuffer.pData = nullptr; //unset to prevent issues
		}
	}
}

#endif //__MESH_H__
