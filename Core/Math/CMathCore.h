#pragma once
#include "pch.h"
#include "CVector.h"


/*
    Use of inlines

    CMath makes heavy use of forceinline, meaning that there are many definitions of one function,
    These will appear in different translation units.

    These have the same address (In memory) in every translation unit.

    These exist because of the way that creative engine is compiled and linked.
    There are three components that make creative engine function, the CECore, which contains all of the math, DX Code,
    and utilities, the second component manages the actual application, and model which controls the drawing and texture conversion.

*/


using namespace CMath;

class CMathCore
{

};

