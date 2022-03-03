#pragma once

#include "CreativeMacros.h"
#include "../resource.h"

// CObject, this is the base object class for all Classes inside Creative - DO NOT Instantiate this directly.
//
// This contains implementations for how objects of this order should copied and moved.

class CRCoreAPI CObject
{
protected:   
	CObject()
	{

	}
	~CObject() = default;


protected:
	//Directions for how copying should behave
	CObject(CObject const& baseObject) = default;
	CObject& operator= (CObject const& baseObject);

protected:

	CObject(CObject&& baseObject) = default;
	CObject& operator= (CObject&& baseObject);

protected:

	//Methods called during program execution
	virtual void Tick(float DeltaTime) = 0;
	virtual void Tick() = 0;
	virtual void DeIntialize() = 0;

};

