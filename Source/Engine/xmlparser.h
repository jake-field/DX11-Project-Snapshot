#pragma once
#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

//Library Includes
#include <windows.h>
#include <vector>
#include <string>

//Types
struct TXMLAttribute
{
	//Member Variables
	std::string strID;
	std::string strValue; //TODO: change this to a union value with a type header instead for interp at load instead of using the conversion functions?

	//Member Functions
	TXMLAttribute(): strID(""), strValue("")
	{}
	TXMLAttribute(const std::string& _strID, const std::string& _strValue): strID(_strID), strValue(_strValue)
	{}
	bool AsBoolean()
	{
		return(strValue == "true");
	}
	int AsInteger()
	{
		return(atoi(strValue.c_str()));
	}
	double AsDouble()
	{
		return(atof(strValue.c_str()));
	}
	float AsFloat()
	{
		return(static_cast<float>(atof(strValue.c_str())));
	}
};

struct TXMLNode
{
	//Member Variables
	std::string strID;
	std::string strContent;
	std::vector<TXMLNode> vecChildren;
	std::vector<TXMLAttribute> vecAttributes;

	//Member Functions
	TXMLNode(): strID(""), strContent("")
	{}
	TXMLNode(const std::string& _strID, const std::string& _strContent): strID(_strID), strContent(_strContent)
	{}
	TXMLAttribute* GetAttributeByID(const std::string& _strID)
	{
		TXMLAttribute* pAttribute = 0;

		for(unsigned int i = 0; i < vecAttributes.size(); ++i)
		{
			if(vecAttributes[i].strID == _strID)
			{
				pAttribute = &vecAttributes[i];
				break;
			}
		}

		return(pAttribute);
	}
};

//Prototypes
class CXMLParser
{
	//Member Functions
public:
	CXMLParser();
	~CXMLParser();

	bool Load(const char* _pcFilename);
	bool Save(const char* _pcFilename);
	bool Save();
	bool Reload();

	int GetNodesByID(const char* _pcID, std::vector<TXMLNode*>& _vecResults);
	TXMLNode* GetNodeByID(const char* _pcID);
	TXMLNode* GetRootNode();
	const char* GetFilename() const;

protected:
	bool RecursiveSearch(const char* _pcSearch, TXMLNode* _pParentNode, std::vector<TXMLNode*>& _rFoundList);
	TXMLNode* RecursiveSearch(const char* _pcSearch, TXMLNode* _pParentNode);
	bool RecursiveSave(std::ofstream& _rFileStream, TXMLNode* _pParentNode, int _iTabDepth = 0);

	bool ParseNode(std::ifstream& _rFileStream, TXMLNode* _pParentNode, bool _bIsRoot = false);
	bool ParseContent(std::ifstream& _rFileStream, TXMLNode* _pParentNode);

	//Member Variables
protected:
	const char* m_pcFilename;
	TXMLNode m_tRootNode;
};

#endif //__XML_PARSER_H__