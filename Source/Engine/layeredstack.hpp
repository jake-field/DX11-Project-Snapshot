#pragma once
#ifndef __LAYERED_STACK_H__
#define __LAYERED_STACK_H__

//Types
template <typename T>
struct TLayeredStackNode
{
	//Member Variables
	T nodeData;
	TLayeredStackNode* ptPrevious;
	TLayeredStackNode* ptNext;

	//Member Functions
	TLayeredStackNode(T _object)
		: nodeData(_object)
		, ptPrevious(nullptr)
		, ptNext(nullptr)
	{
		//Constructor
	}

	~TLayeredStackNode()
	{
		//Destructor, restitch neighbours
		Stitch(nullptr, false, false);
	}

	//If no link, assume discard and delete self
	void Stitch(TLayeredStackNode* _ptLink, bool _bBeforeLink, bool _bAutoDelete = true)
	{
		//Unstitch myself and stitch my neighbours together
		if(ptPrevious) ptPrevious->ptNext = ptNext;
		if(ptNext) ptNext->ptPrevious = ptPrevious;

		//Check if link was parsed
		if(_ptLink)
		{
			//Insert myself before the parsed link?
			if(_bBeforeLink)
			{
				//Stitch myself between the parsed node and its left neighbour
				ptPrevious = _ptLink->ptPrevious;
				ptNext = _ptLink;
				_ptLink->ptPrevious = this;
				if(ptPrevious) ptPrevious->ptNext = this;
			}
			else
			{
				//Stitch myself between the parsed node and its right neighbour
				ptPrevious = _ptLink;
				ptNext = _ptLink->ptNext;
				_ptLink->ptNext = this;
				if(ptNext) ptNext->ptPrevious = this;
			}
		}
		else
		{
			//No link parsed, assume discard
			if(_bAutoDelete) delete this;
		}
	}
};

//Prototypes
template <typename T>
class CLayeredStack
{
	//Member Functions
public:
	CLayeredStack();
	~CLayeredStack();

	void AddObject(T _object);
	void PopObject(T _object);
	void BringToFront(T _object);
	void SendToBack(T _object);

	unsigned int GetSize() const;

	T operator[](unsigned int _uiIndex);

	//Member Variables
protected:
	TLayeredStackNode<T>* m_ptFront;
	unsigned int m_uiSize;
};

//Implementation
template <typename T>
CLayeredStack<T>::CLayeredStack()
	: m_ptFront(nullptr)
	, m_uiSize(0)
{
	//Constructor
}

template <typename T>
CLayeredStack<T>::~CLayeredStack()
{
	//Grab the first node
	TLayeredStackNode<T>* pCurrentNode = m_ptFront;
	TLayeredStackNode<T>* pNextNode = pCurrentNode;

	//For every stack node, delete
	for(unsigned int i = 0; i < m_uiSize; ++i)
	{
		pCurrentNode = pNextNode;
		pNextNode = pCurrentNode->ptNext;
		delete pCurrentNode;
		pCurrentNode = nullptr;
	}
}

template <typename T>
void CLayeredStack<T>::AddObject(T _object)
{
	TLayeredStackNode<T>* pCurrentNode = m_ptFront;
	TLayeredStackNode<T>* pNewNode = new TLayeredStackNode<T>(_object);

	//For every stack node
	for(unsigned int i = 0; i <= m_uiSize; ++i)
	{
		//Check if it contains the same nodeData
		if(pCurrentNode && !pCurrentNode->ptNext)
		{
			//Increase size
			++m_uiSize;
			pNewNode->Stitch(pCurrentNode, false);
			break;
		}
		else if(pCurrentNode)
		{
			//Get the next node
			pCurrentNode = pCurrentNode->ptNext;
		}
		else if(!m_ptFront)
		{
			//Increase size
			++m_uiSize;
			m_ptFront = pNewNode;
			break;
		}
		else
		{
			//Null pointer, broken nodeData stack...
			break;
		}
	}
}

template <typename T>
void CLayeredStack<T>::PopObject(T _object)
{
	//Grab the first node
	TLayeredStackNode<T>* pCurrentNode = m_ptFront;

	//For every stack node
	for(unsigned int i = 0; i < m_uiSize; ++i)
	{
		//Check if it contains the same nodeData
		if(pCurrentNode && pCurrentNode->nodeData = _object)
		{
			//Decrease size
			--m_uiSize;

			//Delete the node, it will restitch any neighbours
			delete pCurrentNode;
			break;
		}
		else if(pCurrentNode)
		{
			//Get the next node
			pCurrentNode = pCurrentNode->ptNext;
		}
		else
		{
			//Null pointer, broken nodeData stack...
			break;
		}
	}
}

template <typename T>
void CLayeredStack<T>::BringToFront(T _object)
{
	//Grab the first node
	TLayeredStackNode<T>* pCurrentNode = m_ptFront;

	//For every stack node
	for(unsigned int i = 0; i < m_uiSize; ++i)
	{
		//Check if it contains the same nodeData
		if(pCurrentNode && pCurrentNode->nodeData == _object)
		{
			//Restitch the node to the front.
			if(m_ptFront != pCurrentNode) pCurrentNode->Stitch(m_ptFront, true);
			m_ptFront = pCurrentNode;
			break;
		}
		else if(pCurrentNode)
		{
			//Get the next node
			pCurrentNode = pCurrentNode->ptNext;
		}
		else
		{
			//Null pointer, broken nodeData stack...
			break;
		}
	}
}

template <typename T>
void CLayeredStack<T>::SendToBack(T _object)
{
	//Grab the first node
	TLayeredStackNode<T>* pCurrentNode = m_ptFront;
	TLayeredStackNode<T>* pFoundNode = nullptr;

	//For every stack node
	for(unsigned int i = 0; i < m_uiSize; ++i)
	{
		//Check if it contains the same nodeData
		if(!pFoundNode && pCurrentNode && pCurrentNode->nodeData = _object)
		{
			//Restitch the node to the back.
			pFoundNode = pCurrentNode;
			pCurrentNode = pCurrentNode->ptNext;
		}
		else if(pCurrentNode)
		{
			if(!pCurrentNode->ptNext && pFoundNode)
			{
				//At back
				pFoundNode->Stitch(pCurrentNode, false);
			}
			else
			{
				//Get the next node
				pCurrentNode = pCurrentNode->ptNext;
			}
		}
		else
		{
			//Null pointer, broken nodeData stack or node was already at the back
			break;
		}
	}
}

template <typename T>
unsigned int CLayeredStack<T>::GetSize() const
{
	return(m_uiSize);
}

template <typename T>
T CLayeredStack<T>::operator[](unsigned int _uiIndex)
{
	//Grab the first node
	TLayeredStackNode<T>* pCurrentNode = m_ptFront;

	for(unsigned int i = 0; i < _uiIndex; ++i)
	{
		pCurrentNode = pCurrentNode->ptNext;
	}

	return(pCurrentNode->nodeData);
}

#endif //__LAYERED_STACK_H__
