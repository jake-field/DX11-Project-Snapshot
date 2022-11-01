#pragma once
#ifndef __IINSTANCE_POOL_H__
#define __IINSTANCE_POOL_H__

//Prototype
class IInstancePool
{
	//Member Functions
protected:
	virtual ~IInstancePool() = default;

public:
	virtual unsigned int GetMax() const = 0;
	virtual unsigned int GetValid() const = 0;
	virtual unsigned int GetStride() const = 0;
	virtual ID3D11Buffer* const GetBuffer() = 0;
	virtual bool IsUpdated() const = 0;

};

#endif //__IINSTANCE_POOL_H__
