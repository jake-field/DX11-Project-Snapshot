//Local Includes
#include <Engine\common.h>
#include <Engine\engine.h>
#include <Engine\inputmanager.h>
#include <Engine\logmanager.h>
#include <Engine\assetmanager.hpp>
#include <Engine\renderer.h>
#include <Engine\freecamera.h>
#include <Engine\light.h>
#include <Engine\staticmesh.h>
#include <Engine\staticmeshinstancer.h>
#include <Engine\model.h>
#include <Engine\texture.h>
#include <Engine\defaultshader.h>
#include <Engine\debugshader.h>
#include <Engine\material.h>
#include <Engine\shaderglobals.h>

//This Include
#include "game.h"

//Implementation
CGame::CGame()
	: m_pRenderer(nullptr)
	, m_pDefaultShader(nullptr)
	, m_pDebugShader(nullptr)
	, m_pCamera(nullptr)
	, m_bDebugBB(false)
	, m_pRiggedEntityTest(nullptr)
{
	//Constructor
}

CGame::~CGame()
{
	//Destructor
	for(auto pEntity : m_vecpEntities) SafeDelete(pEntity);
	m_vecpEntities.clear();

	for(auto pInstancer : m_vecpInstancers) SafeDelete(pInstancer);
	m_vecpInstancers.clear();

	//Destroy the asset manager, unloading all assets loaded in
	CAssetManager::DestroyInstance();

	SafeDelete(m_pDefaultShader);
	SafeDelete(m_pDebugShader);
	SafeDelete(m_pCamera);

	//Released by CEngine
	m_pRenderer = nullptr;
}

bool
CGame::Initialize()
{
	m_pRenderer = CEngine::GetInstance().GetRenderer();
	m_pRenderer->SetClearColor(0.77f, 0.9f, 0.95f);

	//Load Asset Manager
	CAssetManager& rAssetManager = CAssetManager::GetInstance();
	rAssetManager.Initialize(m_pRenderer, 5);

	m_pCamera = new CFreeCamera;
	m_pCamera->Initialize(m_pRenderer);
	m_pCamera->SetNearFarPlane(1.0f, 1000.0f);
	m_pCamera->SetFOV(75.0f);
	m_pCamera->SetPosition(0.0f, 20.0f, -60.0f);
	m_pCamera->SetAsActiveCamera();
	m_pCamera->SetMoveSpeed(0.5f);
	m_pCamera->SetRotSpeed(20.0f);

	//Light properties for the sun
	TLightProperties tSunProperties{};
	tSunProperties.vec4Ambient = float4(0.1f, 0.1f, 0.1f, 1.0f);
	tSunProperties.vec4Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	tSunProperties.vec4Specular = float4(0.25f, 0.25f, 0.25f, 1.0f);

	//Create debug shader
	m_pDebugShader = new CDebugShader;
	m_pDebugShader->Initialize(m_pRenderer);

	//Create default shader
	m_pDefaultShader = new CDefaultShader;
	m_pDefaultShader->Initialize(m_pRenderer, m_pCamera);
	m_pDefaultShader->GetSun()->SetPosition(10.0f, 10.0f, 10.0f);
	m_pDefaultShader->GetSun()->LookAt(float3(0.0f, 0.0f, 0.0f));
	m_pDefaultShader->GetSun()->SetDefinition(tSunProperties);

	//Queue up models for loading
	auto pTestRiggedModel = rAssetManager.LoadAsset<CModel>("Resources\\POLYGON_City_Pack\\Characters\\SK_Character_BusinessMan_Shirt.fbx");
	auto pTestScene = rAssetManager.LoadAsset<CModel>("Resources\\POLYGON_City_Pack\\POLYGON_City_Demo_Scene.fbx");

	//Queue up textures for loading
	TMaterial tMaterial01;
	tMaterial01.pDiffuseTex		= rAssetManager.LoadAsset<CTexture>("Resources\\POLYGON_City_Pack\\Textures\\PolygonCity_Texture_03_A.png");
	tMaterial01.pNormalTex		= rAssetManager.LoadAsset<CTexture>("Resources\\POLYGON_City_Pack\\Textures\\PolygonCity_Texture_Normal.png");
	tMaterial01.pSpecularTex	= rAssetManager.LoadAsset<CTexture>("Resources\\POLYGON_City_Pack\\Textures\\PolygonCity_Texture_SPEC.png");
	tMaterial01.pAOTex			= rAssetManager.LoadAsset<CTexture>("Resources\\POLYGON_City_Pack\\Textures\\PolygonCity_Texture_AO.png");
	tMaterial01.bCastShadow		= true;
	tMaterial01.bReceiveShadow	= true;
	tMaterial01.bTransparent	= false;

	TMaterial tMaterial02;
	tMaterial02.pDiffuseTex		= rAssetManager.LoadAsset<CTexture>("Resources\\POLYGON_City_Pack\\Textures\\BillboardAds.png");
	tMaterial02.bCastShadow		= true;
	tMaterial02.bReceiveShadow	= true;
	tMaterial02.bTransparent	= false;

	TMaterial tMaterial03;
	tMaterial03.pDiffuseTex		= rAssetManager.LoadAsset<CTexture>("Resources\\POLYGON_City_Pack\\Textures\\PolygonCity_Texture_Skyline.png");
	tMaterial03.bCastShadow		= false;
	tMaterial03.bReceiveShadow	= false;
	tMaterial03.bTransparent	= true;

	TMaterial tMaterial04;
	tMaterial04.pDiffuseTex		= rAssetManager.LoadAsset<CTexture>("Resources\\water.png");
	tMaterial04.bCastShadow		= false;
	tMaterial04.bReceiveShadow	= true;
	tMaterial04.bTransparent	= false;

	//Default textures
	CTexture* pBlackTex = rAssetManager.LoadAsset<CTexture>("Resources\\black.png");
	CTexture* pWhiteTex = rAssetManager.LoadAsset<CTexture>("Resources\\white.png");
	CTexture* pErrorTex = rAssetManager.LoadAsset<CTexture>("Resources\\uv.png");
	m_pDefaultShader->SetDefaultTextures(pErrorTex, pBlackTex, pWhiteTex);

	//Apply textures to models
	//TODO: put thread checks in pModel to simplify checking this
	while(!AssetLoaded(pTestScene)) Sleep(1); //Sleep/yield to prevent excessive CPU ticks

	//TODO: this function is not ideal
	/*	0 all
		1 glass(em / trans)
		2 billboard
		3 skyline
		4 water
	*/
	pTestScene->SetMaterial(0, tMaterial01);
	pTestScene->SetMaterial(1, tMaterial01);
	pTestScene->SetMaterial(2, tMaterial02);
	pTestScene->SetMaterial(3, tMaterial03);
	pTestScene->SetMaterial(4, tMaterial04);

	auto n = pTestRiggedModel->GetMaterialCount();
	pTestRiggedModel->SetMaterial(0, tMaterial01);

	//Load up instances into individual meshes for easier testing
	/*for(unsigned int i = 0; i < pTestScene->GetInstanceCount(); ++i)
	{
		CStaticMesh* pMesh = new CStaticMesh;
		ERRORCHECK(pMesh->Initialize(pTestScene, i));
		m_vecpEntities.push_back(pMesh);
	}*/

	//Instance generation
	//m_vecpInstancers
	unsigned int uiMeshCount = pTestScene->GetMeshCount();
	unsigned int uiInstanceCount = pTestScene->GetInstanceCount();
	int* piMeshInstances = new int[uiMeshCount];
	ZeroMemory(piMeshInstances, sizeof(int)* uiMeshCount);

	//Figure out how many of each mesh to instance
	for(unsigned int i = 0; i < uiInstanceCount; ++i)
	{
		//Count up on instances per mesh
		piMeshInstances[pTestScene->GetInstance(i).uiMeshID] += 1;
	}

	//Create instancers
	for(unsigned int i = 0; i < uiMeshCount; ++i)
	{
		CStaticMeshInstancer* pInstancer = new CStaticMeshInstancer;
		m_vecpInstancers.push_back(pInstancer);

		//Init with number of fixed instances of the mesh in the model
		pInstancer->Initialize(m_pRenderer, piMeshInstances[i]);
		pInstancer->ReadyBatch();
	}

	//Create the meshes for the instancer
	for(unsigned int i = 0; i < uiInstanceCount; ++i)
	{
		//Get the relevant instancer for the instance mesh
		CStaticMeshInstancer* pInstancer = m_vecpInstancers[pTestScene->GetInstance(i).uiMeshID];

		//Create new mesh and add it to the entity list
		CStaticMesh* pMesh = new CStaticMesh;
		m_vecpEntities.push_back(pMesh);

		//TODO: don't batch meshes where there are less than a certain number as instances need more than 1? instances for the mesh?

		//Init the mesh with the instance data
		pMesh->Initialize(pTestScene, i, pInstancer);

		//Add the mesh to the instancer
		pInstancer->AddToBatch(pMesh);
	}

	//Close instance buffers
	for(auto pInstancer : m_vecpInstancers) pInstancer->FinishBatch();
	SafeDeleteArray(piMeshInstances);

	auto pHuman = new CStaticMesh;
	pHuman->Initialize(pTestRiggedModel);
	m_pRiggedEntityTest = pHuman;

	//TODO: this will process here, but when we make an actual world we will need to consider better isolation for rendering
	//		especially with the instancers etc.
	//		This is fine for processing, but rendering gets a bit finnicky with wasted loops/performance
	m_vecpEntities.push_back(m_pRiggedEntityTest); //add it to the list for processing

	return false;
}

void
CGame::Process(float _fDeltaTick)
{
	//Input
	CInputManager& rInput = CInputManager::GetInstance();

	//Process main camera (freecam)
	m_pCamera->Process(_fDeltaTick);

	//Debug test for showing bounding boxes
	if(rInput.IsPressed(VK_F1))
	{
		m_bDebugBB = !m_bDebugBB;
		rInput.SetKeyboardInput(VK_F1, false);
	}

	//Debug test for flipping render modes between Solid and Wireframe
	if(rInput.IsPressed(VK_F2))
	{
		m_pRenderer->SetRenderMode(!m_pRenderer->GetRenderMode());
		rInput.SetKeyboardInput(VK_F2, false);
	}

	//Sun demo rotation
	static float sfTime = 0.0f;
	sfTime += _fDeltaTick * 10.0f;
	m_pDefaultShader->GetSun()->SetRotation(45.0f, sfTime, 0.0f);

	//Process all entities
	for(auto pEntity : m_vecpEntities) pEntity->Process(_fDeltaTick);

	//Calculate scene shadows
	//TODO: update  this func to use quad-trees and CSM
	m_pDefaultShader->CalculateSceneShadow(m_vecpEntities);

	//update globals using main camera
	m_pRenderer->UpdateGlobalCBuffer(m_pCamera);
}

void
CGame::Draw()
{
	//Default Render
	for(int iPass = 0; iPass < 2; ++iPass)
	{
		//First pass apply shader
		if(!iPass) m_pDefaultShader->ApplyShader();
		else m_pDefaultShader->SetPass(iPass);

		//Draw static instance batches
		for(auto pInstancer : m_vecpInstancers) pInstancer->DrawBatch();

		//draw our single human
		m_pRiggedEntityTest->Draw();
	}

	//Debug
	if(m_bDebugBB)
	{
		//Draw all meshes
		//TODO: Make the debugshader use an instancer
		m_pDebugShader->ApplyShader();
		for(auto pEntity : m_vecpEntities)
		{
			auto tBB = pEntity->GetOBB();
			m_pDebugShader->DrawCube(tBB.Center, tBB.Orientation, (float3)tBB.Extents * 2.0f);
		}
	}
}
