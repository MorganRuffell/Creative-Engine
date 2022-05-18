#pragma once

#include "../Core/VectorMath.h"

/// <summary>
/// 
///		CJoint is a skeletal joint between two peices, if you're unsure about this refer to the skeletal mesh
///		on the unreal mannequin. 
/// 
///		This has Physics constraints, that in future I'll code into an options based system
///		that will allow for some super coool stuff like angular velocity, torque... AHHHHHH I'm geeking out just		thinking about it!
/// 
///		I digress, but these are the bones that make up a skinned mesh. Sorry everyone!
/// </summary>

enum class EConstraintType
{
	BallNSocket,
	Hinge,
	Prismatic
};

struct CJoint
{
public:

	CMath::CMatrix4 posXform;
	CMath::CMatrix3 nrmXform;


	EConstraintType GetConstraintType()
	{
		return m_ConstraintType;
	}


protected:

	EConstraintType m_ConstraintType;

private:

	bool IsRoot;
};

//Prototype for join based strcture, maybe a full skeleton?
class CJointStructure
{

};
