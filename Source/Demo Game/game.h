#pragma once
#ifndef __GAME_H__
#define __GAME_H__

//Library Includes
#include <vector>
#include <Engine\gametemplate.h>

//Prototypes
class CRenderer;
class CDefaultShader;
class CDebugShader;
class CFreeCamera;
class CEntity3D;
class CStaticMeshInstancer;
class CLight;
class CGame: public IGameTemplate<CGame>
{
	//Member Functions
protected:
	CGame();
	~CGame();
	friend IGameTemplate; //To allow the singleton template to work

public:
	bool Initialize();
	void Process(float _fDeltaTick);
	void Draw();

	//Member Variables
protected:
	CRenderer* m_pRenderer;

	CDefaultShader* m_pDefaultShader;
	CDebugShader* m_pDebugShader;

	CEntity3D* m_pRiggedEntityTest;
	std::vector<CEntity3D*> m_vecpEntities;
	std::vector<CStaticMeshInstancer*> m_vecpInstancers;

	CFreeCamera* m_pCamera;

	bool m_bDebugBB;

};

#endif //__GAME_H__
