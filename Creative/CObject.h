#pragma once

#include "Source/CCore/CreativeMacros.h"
#include "Resource.h"

// CObject, this is the base object class for all Classes inside Creative - DO NOT Instantiate this directly.
//
// This contains implementations for how objects of this order should copied and moved.

class CRCoreAPI CObject
{
protected:
	CObject();
	~CObject() = default;


protected:
	//Directions for how copying should behave
	CObject(CObject const& baseObject) = default;
	virtual CObject& operator= (CObject const& baseObject) = 0;

protected:

	CObject(CObject&& baseObject) = default;
	virtual CObject& operator= (CObject&& baseObject) = 0;

protected:

	//Methods called during program execution
	virtual bool Intialize(HWND hWnd, HINSTANCE hInst) = 0;
	virtual void Tick(float DeltaTime) = 0;
	virtual void Tick() = 0;
	virtual void DeIntialize() = 0;

};

