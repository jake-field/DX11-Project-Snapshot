#pragma once
#ifndef __SHADER_SCENE_H__
#define __SHADER_SCENE_H__

//Local Includes
#include "dx11shader.h"
#include "light.h" //Used in the cbuffer
#include "shaderglobals.h"

//Prototypes
class CCamera;
class CTexture;
class CRenderer;
class CDefaultShader: protected CDX11Shader
{
	//Member Functions
public:
	CDefaultShader();
	~CDefaultShader();

	bool Initialize(CRenderer* _pRenderer, CCamera* _pSceneCamera);
	bool ApplyShader(int _iPass = 0);
	bool SetPass(int _iPass);
	void FinishShader();

	bool Predraw(const IMesh* _pMesh, const float4x4* _ptWorldMatrix, bool _bInstanced = false);

	//Default Textures
	void SetDefaultTextures(CTexture* _pError, CTexture* _pBlack = nullptr, CTexture* _pWhite = nullptr);

	//Used to update the focus of the shadowmapping
	void CalculateSceneShadow(std::vector<CEntity3D*> _vecpEntities);

	//Allows external modification of the sun
	//TODO: Consider external sun object or a light handler which can return a selection of lights in the frustum
	CLight* GetSun() const;

private:
	//Not used
	bool Initialize(CRenderer* _pRenderer) { return(false); };

	//Member Variables
protected:
	CCamera* m_pSceneCamera; //Main Scene Camera
	CLight* m_pSunLight; //Sun
	int m_iActivePass; //Storage of what pass we are working with

	//For missing textures
	CTexture* m_pErrorTex;
	CTexture* m_pBlackTex;
	CTexture* m_pWhiteTex;

	//Constant Buffer
	ID3D11Buffer** m_pCBuffers;
	int m_iCBufferCount;

	//Shader Resources
	ID3D11Texture2D* m_pShadowMapTexture;
	ID3D11DepthStencilView* m_pShadowMapDSV;
	ID3D11ShaderResourceView* m_pShadowMapSRV;
	ID3D11RasterizerState* m_prsShadow;

	//Confined struct declarations
protected:
	struct TCBufferScenePerFrame
	{
		struct TShadowMapInformation
		{
			float4x4 matSunVP;
			float3 vec3SunPos; float fPad0;
			float3 vec3SunDir; float fPad1;
			TLightProperties tSunProperties;
		} tSun;
	};

	struct TCBufferScenePerObject
	{
		float4x4 matWorld;
		bool bRenderUnlit; bool bPadding[3]; //Render without using lighting
		float3 padding;
		//TODO: Consider bRecieveShadows for rendering meshes with normal lighting but unaffected by shadowmap tests?
		//bCastShadow check is done on the cpu (Simply not draw to pass0)
		//TODO: Consider a separate cbuffer for pass0 if the size of this cbuffer gets too large, for now it's small enough not to worry
	};

	enum class EDefaultShaderBindings
	{
		CB_PERFRAME			= ShaderGlobals::CB_UsableStartSlot,
		CB_PEROBJECT, //Next after per-frame slot
		TX_DIFF_NRM_SPEC_AO = ShaderGlobals::TX_DIFFUSE, //Start slot at diffuse
		TX_SHADOWMAP		= ShaderGlobals::TX_SHADOWMAP, //1..4 taken by TX_DIFF_NRM_SPEC_AO
	};
};

#endif //__SHADER_SCENE_H__
