
#include "pch.h"
#include "CBoundingSphere.h"

using namespace CMath;

CBoundingSphere CBoundingSphere::Union( const CBoundingSphere& rhs )
{
    float radA = GetRadius();
    if (radA == 0.0f)
        return rhs;

    float radB = rhs.GetRadius();
    if (radB == 0.0f)
        return *this;

    CVector3 diff = GetCenter() - rhs.GetCenter();
    float dist = Length(diff);

    // Safe normalize vector between sphere centers
    diff = dist < 1e-6f ? CVector3(kXUnitVector) : diff * Recip(dist);

    CVector3 extremeA = GetCenter() + diff * Max(radA, radB - dist);
    CVector3 extremeB = rhs.GetCenter() - diff * Max(radB, radA - dist);

    return CBoundingSphere((extremeA + extremeB) * 0.5f, Length(extremeA - extremeB) * 0.5f);
}
