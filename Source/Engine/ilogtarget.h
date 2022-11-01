#pragma once
#ifndef __LOG_TARGET_H__
#define __LOG_TARGET_H__

//Prototype
class ILogTarget
{
	//Member Functions
protected:
	ILogTarget() = default;
	ILogTarget(const ILogTarget&) = delete;

public:
	virtual ~ILogTarget() = default;
	virtual void Write(const char* _pcstr) = 0;
	virtual void WriteLine(const char* _pcstr) = 0;
	virtual const char* GetID() const = 0;

};

#endif //__LOG_TARGET_H__