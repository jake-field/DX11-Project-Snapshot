#pragma once
#ifndef __SHADER_DEBUG_H__
#define __SHADER_DEBUG_H__

//Local Includes
#include "dx11shader.h"
#include "mesh.hpp"

//Prototypes
class CRenderer;
class CDebugShader: protected CDX11Shader
{
	//Member Functions
public:
	CDebugShader();
	~CDebugShader();

	bool Initialize(CRenderer* _pRenderer);
	bool ApplyShader();
	void FinishShader();

	//TODO: use passes for swapping GS, consider instancing support for 3D objects
	//		also consider GS for line, quads, circles, theoretical max for debug implementation would be 146 vertices per point
	//		This would cover 3D objects too but quickly complicate branch evaluation, which will probably require multiple GS
	//		GS max limits: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dcl-maxoutputvertexcount
	//void DrawLine();
	//void DrawPlane();
	void DrawCube(float3 _vec3pos, float3 _vec3rot, float3 _vec3scale);
	void DrawCube(float3 _vec3pos, float4 _quatRot, float3 _vec3scale);
	void DrawSphere(float3 _vec3pos, float3 _vec3rot, float3 _vec3scale);
	void DrawSphere(float3 _vec3pos, float3 _vec3rot, float _fRadius);
	//void DrawCapsule();
	//void DrawCircle();
	//void DrawPoly();

	//Direct drawing of primitives
	bool Predraw(const IMesh* _pMesh, const float4x4* _ptWorldMatrix, bool _bInstanced = false);

private:
	//Not used, default or empty
	bool ApplyShader(int _iPass) { return(__super::ApplyShader(_iPass)); };
	bool SetPass(int _iPass) { return(__super::SetPass(_iPass)); };

	//Member Variables
protected:
	//Constant Buffer
	ID3D11Buffer** m_pCBuffers;
	int m_iCBufferCount;

	bool m_bWasWireframe;

	//Meshes
	CMesh<float3> m_meshSphere;
	CMesh<float3> m_meshCube;

	//Confined struct declarations
protected:

	struct TCBufferDebugPerObject
	{
		float4x4 matWorld;
	};

	enum class EDebugShaderBindings
	{
		CB_PEROBJECT = 1,
	};
};

#endif //__SHADER_DEBUG_H__
