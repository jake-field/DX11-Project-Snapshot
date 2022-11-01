#pragma once
#ifndef __GAME_TEMPLATE_H__
#define __GAME_TEMPLATE_H__

//Prototypes
template<class T>
class IGameTemplate
{
	//Member Functions
protected:
	IGameTemplate(): m_bQuit(false) {};
	IGameTemplate(const IGameTemplate&) = default;
	virtual ~IGameTemplate() = default;

public:
	static T& GetInstance()
	{
		if(!T::sm_pGame) T::sm_pGame = new T();
		return(*T::sm_pGame);
	}

	static void DestroyInstance()
	{
		if(sm_pGame)
		{
			delete sm_pGame;
			sm_pGame = nullptr;
		}
	}

	virtual bool Initialize() = 0;
	virtual void Process(float _fDeltaTick) = 0;
	virtual void Draw() = 0;

	static bool _Process(float _fDeltaTick)
	{
		//Process
		T::GetInstance().Process(_fDeltaTick);
		return(!T::sm_pGame->m_bQuit); //If false, exit
	}

	static void _Draw()
	{
		T::GetInstance().Draw();
	}

	//Member Variables
protected:
	static T* sm_pGame;
	bool m_bQuit;
};

template<class T> T* IGameTemplate<T>::sm_pGame = nullptr;

#endif //__GAME_TEMPLATE_H__