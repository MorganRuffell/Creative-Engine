#include "CMathCore.h"

class CRGraphicsAPI CVertexBase
{
public:

	SFloat3 GetPosition()
	{
		return Position;
	}

	bool SetPosition(SFloat3 NewPosition)
	{
		Position = NewPosition;
		return true;
	}

	bool ResetToWorldOrigin()
	{
		if (SetPosition(Origin))
		{
			return true;
		}

		return false;
	}

	bool SetWorldOrigin(SFloat3 NewOrigin)
	{
		if (NewOrigin.x != Origin.x)
		{
			if (NewOrigin.y != Origin.y)
			{
				if (NewOrigin.z != Origin.z)
				{
					Origin = NewOrigin;
					return true;
				}
			}
		}

		return false;
	}


protected:

	SFloat3 Position;
	SFloat4 color;

	SFloat2 TexCoordinates;

private:

	SFloat3 Normal;
	SFloat3 Tangent;

	SFloat2 Texture0;
	SFloat2 Texture1;

	const SFloat3 worldZero = {0.0f,0.0f,0.0f};

	SFloat3 Origin;
};

