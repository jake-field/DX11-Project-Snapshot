//Library Includes
#include <fstream>
#include <string>
#include <vector>

//Local Includes
#include "common.h"

//This Include
#include "xmlparser.h"

//Implementation
CXMLParser::CXMLParser()
{
	//Constructor
}

CXMLParser::~CXMLParser()
{
	//Destructor
}

bool
CXMLParser::Load(const char* _pcFilename)
{
	//Return variable
	bool bSuccessful = false;

	//Only attempt the load if we recieved a filename
	if(_pcFilename != 0 && _pcFilename != "")
	{
		//Store the last filename
		m_pcFilename = _pcFilename;

		//Open the file and load each element and it's children
		std::ifstream theFile;
		theFile.open(m_pcFilename);
		m_tRootNode = TXMLNode("", "");
		bSuccessful = ParseNode(theFile, &m_tRootNode, true);
		theFile.close();
	}
	else
	{
		//No filename was passed through
	}

	//Return true if XML loaded properly
	return(bSuccessful);
}

bool
CXMLParser::Save(const char* _pcFilename)
{
	//Return variable
	bool bSuccessful = false;

	//Only attempt the save if we recieved a filename
	if(_pcFilename != 0 && _pcFilename != "")
	{
		//Store the last filename
		m_pcFilename = _pcFilename;

		//Open the file and save each element and it's children
		std::ofstream theFile;
		theFile.open(m_pcFilename);
		bSuccessful = RecursiveSave(theFile, &m_tRootNode);
		theFile.close();
	}
	else
	{
		//No filename was passed through
	}

	//Returns true if the stored XML data saved successfully
	return(bSuccessful);
}

bool
CXMLParser::Save()
{
	//Quick save function that overwrites the open file
	return(Save(m_pcFilename));
}

bool
CXMLParser::Reload()
{
	//Quick reload function that reloads the open file
	return(Load(m_pcFilename));
}

int
CXMLParser::GetNodesByID(const char* _pcID, std::vector<TXMLNode*>& _vecResults)
{
	//Looks for any nodes in the tree with the same ID
	RecursiveSearch(_pcID, &m_tRootNode, _vecResults);
	return(_vecResults.size());
}

TXMLNode*
CXMLParser::GetNodeByID(const char* _pcID)
{
	return(RecursiveSearch(_pcID, &m_tRootNode));
}

TXMLNode*
CXMLParser::GetRootNode()
{
	//Returns the root node
	return(&m_tRootNode);
}

const char*
CXMLParser::GetFilename() const
{
	//Returns the filename if the XML document has interacted with the system
	return(m_pcFilename);
}

bool
CXMLParser::RecursiveSearch(const char* _pcSearch, TXMLNode* _pParentNode, std::vector<TXMLNode*>& _rFoundList)
{
	//Return variable
	bool bFoundSomething = false;

	//For each child node that the parent node has
	for(unsigned int i = 0; i < _pParentNode->vecChildren.size(); ++i)
	{
		//Compare ID with Search
		if(_pParentNode->vecChildren[i].strID == _pcSearch)
		{
			//Add node to the list
			_rFoundList.push_back(&_pParentNode->vecChildren[i]);
			bFoundSomething = true;
		}
		else
		{
			//Different ID than what we are looking for
		}

		//Do a recursive check on the child and see if we found something
		bFoundSomething = (bFoundSomething || RecursiveSearch(_pcSearch, &_pParentNode->vecChildren[i], _rFoundList));
	}

	//Returns true if at least one item was found
	return(bFoundSomething);
}

TXMLNode*
CXMLParser::RecursiveSearch(const char* _pcSearch, TXMLNode* _pParentNode)
{
	//Return variable
	TXMLNode* ptNode = 0;

	//For each child node that the parent node has
	for(unsigned int i = 0; i < _pParentNode->vecChildren.size(); ++i)
	{
		//Compare ID with Search
		if(_pParentNode->vecChildren[i].strID == _pcSearch)
		{
			//Add node to the list
			ptNode = &_pParentNode->vecChildren[i];
			break;
		}
		else
		{
			//Different ID than what we are looking for
		}

		//Do a recursive check on the child and see if we found something
		ptNode = RecursiveSearch(_pcSearch, &_pParentNode->vecChildren[i]);
		if(ptNode) break;
	}

	//Returns true if the item was found
	return(ptNode);
}

bool
CXMLParser::RecursiveSave(std::ofstream& _rFileStream, TXMLNode* _pParentNode, int _iTabDepth)
{
	//Function variables
	bool bSuccessful = true;
	bool bNoContent = false;
	char* pcTabDepth = new char[_iTabDepth + 1];

	//Build tab string
	for(int i = 0; i < _iTabDepth; ++i)
	{
		pcTabDepth[i] = '\t';
	}

	//Append a terminator character to the end of the tab string to fix junk data
	pcTabDepth[_iTabDepth] = '\0';

	//Check if the file was open
	if(_rFileStream.is_open())
	{
		//Print node tag
		_rFileStream << pcTabDepth << "<" << _pParentNode->strID;

		//Print attributes
		for(unsigned int i = 0; i < _pParentNode->vecAttributes.size(); ++i)
		{
			_rFileStream << " " << _pParentNode->vecAttributes[i].strID << "=\"" << _pParentNode->vecAttributes[i].strValue << "\"";
		}

		//Check to see if we should end the node on the same line
		if(!_pParentNode->strContent.empty())
		{
			//Print content with an extra tab (for clean indenting) after closing the tag
			_rFileStream << ">" << std::endl << pcTabDepth << "\t" << _pParentNode->strContent << std::endl;
		}
		else if(_pParentNode->vecChildren.empty())
		{
			//No body to this node, close it on this line
			_rFileStream << " />" << std::endl;
			bNoContent = true;
		}
		else
		{
			//Close the tag, no content but we have children
			_rFileStream << ">" << std::endl;
		}

		//Print children, if any
		for(unsigned int i = 0; i < _pParentNode->vecChildren.size(); ++i)
		{
			bSuccessful = bSuccessful && RecursiveSave(_rFileStream, &_pParentNode->vecChildren[i], (_iTabDepth + 1));
		}

		//Close node if there was content, also avoid print a blank newline at the end of the file to prevent future errors
		if(!bNoContent) _rFileStream << pcTabDepth << "</" << _pParentNode->strID << ">";
		if(!bNoContent && _pParentNode != &m_tRootNode) _rFileStream << std::endl;
	}
	else
	{
		//File not open, failed to execute function
		bSuccessful = false;
	}

	//Clean up
	if(pcTabDepth) SafeDeleteArray(pcTabDepth);

	//Return true if all recursive functions and this one were successful
	return(bSuccessful);
}

bool
CXMLParser::ParseNode(std::ifstream& _rFileStream, TXMLNode* _pParentNode, bool _bIsRoot)
{
	//Function Variables
	bool bSuccessful = true;
	bool bNoContent = false;
	bool bNoAttributes = false;
	std::string strBlock = "";
	TXMLNode tMyNode;
	TXMLNode* pWorkingNode = &tMyNode;

	if(_bIsRoot) pWorkingNode = &m_tRootNode;

	//Check if the file was open
	if(_rFileStream.is_open())
	{
		//Get tag name
		strBlock = "";
		char cChar = _rFileStream.get();
		while((cChar != ' ' && cChar != '>') && !_rFileStream.eof())
		{
			if(cChar != '<') pWorkingNode->strID += cChar;
			cChar = _rFileStream.get();
		}

		if(cChar == '>') bNoAttributes = true;

		if(cChar == ' ' && _rFileStream.peek() == '/')
		{
			while(_rFileStream.get() != '>' && !_rFileStream.eof());
			bNoAttributes = true;
			bNoContent = true;
		}

		//Get attributes
		if(!bNoAttributes)
		{
			strBlock = "";
			std::string strAttribID = "";
			bool bIsString = false;

			while(!_rFileStream.eof())
			{
				if(cChar == '=')
				{
					strAttribID = strBlock;
					strBlock = "";
				}
				else if((cChar == '\"' && _rFileStream.peek() == ' ') || cChar == '>' || (cChar == '/' && _rFileStream.peek() == '>'))
				{
					if((cChar == '\"' && _rFileStream.peek() == ' ')) strBlock += cChar;
					if(!strAttribID.empty() && !strBlock.empty()) pWorkingNode->vecAttributes.push_back(TXMLAttribute(strAttribID.substr(1), strBlock.substr(1, strBlock.size() - 2)));

					strAttribID = "";
					strBlock = "";

					if(cChar == '/' && _rFileStream.peek() == '>')
					{
						bNoContent = true;
						break;
					}
					else if(cChar == '>')
					{
						bNoContent = false;
						break;
					}
				}
				else
				{
					strBlock += cChar;
				}

				cChar = _rFileStream.get();
			}
		}
		else
		{
			//Attributes found
		}

		//Get contents
		if(!bNoContent) bSuccessful = bSuccessful && ParseContent(_rFileStream, pWorkingNode);

		//Store myself
		if(!_bIsRoot) _pParentNode->vecChildren.push_back(*pWorkingNode);
	}
	else
	{
		//File not open, failed to execute function
		bSuccessful = false;
	}

	//Return true if all recursive functions and this one were successful
	return(bSuccessful);
}

bool
CXMLParser::ParseContent(std::ifstream& _rFileStream, TXMLNode* _pParentNode)
{
	bool bSuccesful = true;
	char cChar = _rFileStream.get();

	//Read the content
	while(cChar != '<' && !_rFileStream.eof())
	{
		if(cChar != '\t' && cChar != '\n') _pParentNode->strContent += cChar;
		cChar = _rFileStream.get();
	}

	//Check if we hit the end of our node
	if(_rFileStream.peek() != '/' && cChar == '<')
	{
		//Parse the node that we found
		bSuccesful = bSuccesful && ParseNode(_rFileStream, _pParentNode);

		//Check to see if ParseNode accidentally left the end of a tag in the stream
		cChar = _rFileStream.get();
		if(cChar == '>') cChar = _rFileStream.get();

		//Keep parsing the rest of the content until the end tag is found
		bSuccesful = bSuccesful && ParseContent(_rFileStream, _pParentNode);
	}
	else
	{
		//Found the end of the content, skip the closing tag
		while(_rFileStream.get() != '>' && !_rFileStream.eof());
	}

	//Return true if successful
	return(bSuccesful);
}