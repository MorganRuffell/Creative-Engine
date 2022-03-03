#pragma once
#include "CObject.h"
#include "CMathCore.h"


class CCameraCore
{
public:

	CCameraCore(SFloat3 GlobalPosition);
	~CCameraCore();

public:




protected:

	virtual void BeginRoll(SFloat3 position);

	//DeltaTime is seconds elapsed
	virtual void Update(float DeltaTime);

	virtual void Reset();

	SFloat3 Local_initialPosition;

};

