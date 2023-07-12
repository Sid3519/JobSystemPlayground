#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <math.h>

//--------------------------------------------------------------------------------------------------

RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius)
{
	RaycastResult2D raycastResult2D;

	Vec2& iBasis = fwdNormal;
	Vec2 jBasis = fwdNormal.GetRotated90Degrees();
	Vec2 dispFromStartToDiscCen = discCenter - startPos;
	float projectionOfDispAlongiBasis = DotProduct2D(dispFromStartToDiscCen, iBasis);
	float projectionOfDispAlongjBasis = DotProduct2D(dispFromStartToDiscCen, jBasis);
	
	float projectionAlongjBasisSquared = projectionOfDispAlongjBasis * projectionOfDispAlongjBasis;
	float discRadiusSquared = discRadius * discRadius;	
	// raycast is above/below the disc
	if (projectionAlongjBasisSquared > discRadiusSquared)
	{
		return raycastResult2D;
	}

	// raycast is too far to left or right of the disc
	if (projectionOfDispAlongiBasis <= -discRadius ||
		projectionOfDispAlongiBasis >= (maxDist + discRadius))
	{
		return raycastResult2D;
	}

	// raycast starts inside the disc
	if (IsPointInsideDisc2D(startPos, discCenter, discRadius))
	{
		raycastResult2D.m_didImpact = true;
		raycastResult2D.m_impactDist = 0.f;
		raycastResult2D.m_impactNormal = -iBasis;
		raycastResult2D.m_impactPos = startPos;

		return raycastResult2D;
	}

	// raycast is either a tangent or outside the disc
	float halfChordLengthSquared = discRadiusSquared - projectionAlongjBasisSquared;
	if (halfChordLengthSquared <= 0)
	{
		return raycastResult2D;
	}

	// raycast didn't reach the disc or raycast impact is before the raycast startpos
	float halfChrodLength = sqrtf(halfChordLengthSquared);
	float impactDist = projectionOfDispAlongiBasis - halfChrodLength;
	if (impactDist >= maxDist || impactDist <= 0)
	{
		return raycastResult2D;
	}

	raycastResult2D.m_didImpact = true;
	raycastResult2D.m_impactDist = impactDist;
	Vec2 impactDistVec = impactDist * iBasis;
	raycastResult2D.m_impactPos = startPos + impactDistVec;
	Vec2 dispFromDiscCenToImpactPos = raycastResult2D.m_impactPos - discCenter;
	raycastResult2D.m_impactNormal = dispFromDiscCenToImpactPos.GetNormalized();

	return raycastResult2D;
}

RaycastResult2D RaycastVsAABB2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist, AABB2 const& bounds)
{
	RaycastResult2D raycastResult2D;

	if (bounds.IsPointInside(startPos))
	{
		raycastResult2D.m_didImpact = true;
		raycastResult2D.m_impactDist = 0.f;
		raycastResult2D.m_impactNormal = -fwdNormal;
		raycastResult2D.m_impactPos = startPos;

		return raycastResult2D;
	}

	Vec2 const& iBasis = fwdNormal;
	Vec2 const& boundMins = bounds.m_mins;
	Vec2 const& boundMaxs = bounds.m_maxs;

	float inverseIBasisX = 1.f / iBasis.x;
	float enterTimeX = (boundMins.x - startPos.x) * inverseIBasisX;
	float exitTimeX = (boundMaxs.x - startPos.x) * inverseIBasisX;

	float inverseIBasisY = 1.f / iBasis.y;
	float enterTimeY = (boundMins.y - startPos.y) * inverseIBasisY;
	float exitTimeY = (boundMaxs.y - startPos.y) * inverseIBasisY;

	FloatRange overlapTimeX = enterTimeX < exitTimeX ? FloatRange(enterTimeX, exitTimeX) : FloatRange(exitTimeX, enterTimeX);
	FloatRange overlapTimeY = enterTimeY < exitTimeY ? FloatRange(enterTimeY, exitTimeY) : FloatRange(exitTimeY, enterTimeY);
	
	if (!overlapTimeX.IsOverlappingWith(overlapTimeY))
	{
		return raycastResult2D;
	}

	float enterTime = overlapTimeX.m_min < overlapTimeY.m_min ? overlapTimeY.m_min : overlapTimeX.m_min;
	if (enterTime > maxDist || enterTime < 0.f)
	{
		return raycastResult2D;
	}

	Vec2 impactPos = startPos + (enterTime * iBasis);
	float imapactDist = (impactPos - startPos).GetLength();
	Vec2 impactNormal = enterTime == overlapTimeX.m_min ? Vec2(-(impactPos.x - startPos.x), 0.f).GetNormalized() : Vec2(0.f, -(impactPos.y - startPos.y)).GetNormalized();
	raycastResult2D.m_didImpact = true;
	raycastResult2D.m_impactDist = imapactDist;
	raycastResult2D.m_impactNormal = impactNormal;
	raycastResult2D.m_impactPos = impactPos;

	return raycastResult2D;
}

//--------------------------------------------------------------------------------------------------

RaycastResult2D RaycastVsLineSegment2D(Vec2 const& rayStartPos, Vec2 const& fwdNormal, float maxDist, Vec2 const& lineSegmentStartPos, Vec2 const& lineSegmentEndPos)
{
	RaycastResult2D raycastResult2D;

	Vec2 const& iBasis = fwdNormal;
	Vec2 jBasis = fwdNormal.GetRotated90Degrees();

	Vec2 dispRayStartToLineSegEnd = lineSegmentEndPos - rayStartPos;
	float dispRayStartToLineSegEndProjectedLengthAlongJBasis = DotProduct2D(dispRayStartToLineSegEnd, jBasis);

	Vec2 dispRayStartToLineSegStart = lineSegmentStartPos - rayStartPos;
	float dispRayStartToLineSegStartProjectedLengthAlongJBasis = DotProduct2D(dispRayStartToLineSegStart, jBasis);

	// Straddle check
	if ((dispRayStartToLineSegStartProjectedLengthAlongJBasis <= 0 && dispRayStartToLineSegEndProjectedLengthAlongJBasis <= 0) ||
		dispRayStartToLineSegStartProjectedLengthAlongJBasis >= 0 && dispRayStartToLineSegEndProjectedLengthAlongJBasis >= 0)
	{
		return raycastResult2D;
	}
	
	float impactFraction = -dispRayStartToLineSegStartProjectedLengthAlongJBasis / (-dispRayStartToLineSegStartProjectedLengthAlongJBasis + dispRayStartToLineSegEndProjectedLengthAlongJBasis);
	Vec2 dispLineSegmentStartToEnd = lineSegmentEndPos - lineSegmentStartPos;
	Vec2 impactPos = lineSegmentStartPos + (impactFraction * dispLineSegmentStartToEnd);
	Vec2 dispRayStartToImpactPos = impactPos - rayStartPos;
	float impactDist = DotProduct2D(dispRayStartToImpactPos, iBasis);

	// ray starts before the line segment || ray did not reach the line segment
	if (impactDist >= maxDist || impactDist <= 0)
	{
		return raycastResult2D;
	}
	Vec2 impactNormal = dispLineSegmentStartToEnd.GetRotated90Degrees();
	impactNormal.Normalize();
	float impactNormalDir = DotProduct2D(impactNormal, iBasis);
	impactNormal = impactNormalDir >= 0 ? -impactNormal : impactNormal;

	raycastResult2D.m_didImpact = true;
	raycastResult2D.m_impactDist = impactDist;
	raycastResult2D.m_impactNormal = impactNormal;
	raycastResult2D.m_impactPos = impactPos;

	return raycastResult2D;
}

//--------------------------------------------------------------------------------------------------

RaycastResult3D RaycastVsCylinderZ3D(Vec3 const& rayStart, Vec3 const& fwdNormal, float maxDist, Vec2 const& centerXY, FloatRange minMaxZ, float radius)
{
	RaycastResult3D raycastResult3D;

	Vec2 rayStartXY = Vec2(rayStart.x, rayStart.y);
	bool isRayInCylinder = IsPointInsideDisc2D(rayStartXY, centerXY, radius);
	if (isRayInCylinder && minMaxZ.IsOnRange(rayStart.z))
	{
		raycastResult3D.m_didImpact = true;
		raycastResult3D.m_impactDist = 0.f;
		raycastResult3D.m_impactNormal = -fwdNormal;
		raycastResult3D.m_impactPos = rayStart;

		return raycastResult3D;
	}

	Vec3 rayEnd = rayStart + (fwdNormal * maxDist);
	Vec3 rayDispSE = rayEnd - rayStart; // ray displacement from start to end
	Vec2 rayDispSEXY = Vec2(rayDispSE.x, rayDispSE.y);
	float rayDispSEXYLen = rayDispSEXY.GetLength();
	Vec2 fwdNormalXY = Vec2(fwdNormal.x, fwdNormal.y);
	fwdNormalXY.Normalize();
	
	// TODO: should the max distance change to rayDispSEXY's length
	RaycastResult2D raycastResult2D = RaycastVsDisc2D(rayStartXY, fwdNormalXY, rayDispSEXYLen, centerXY, radius);

	if (raycastResult2D.m_didImpact)
	{
		float scaleFactor = (raycastResult2D.m_impactPos.x - rayStart.x) / fwdNormal.x;
		float zImpactPos = rayStart.z + (fwdNormal.z * scaleFactor);
		if (minMaxZ.IsOnRange(zImpactPos))
		{
			raycastResult3D.m_didImpact = true;
			raycastResult3D.m_impactPos.x = raycastResult2D.m_impactPos.x;
			raycastResult3D.m_impactPos.y = raycastResult2D.m_impactPos.y;
			raycastResult3D.m_impactPos.z = zImpactPos;
			raycastResult3D.m_impactDist = (raycastResult3D.m_impactPos - rayStart).GetLength();
			raycastResult3D.m_impactNormal.x = raycastResult2D.m_impactNormal.x;
			raycastResult3D.m_impactNormal.y = raycastResult2D.m_impactNormal.y;
			raycastResult3D.m_impactNormal.z = 0.f;

			return raycastResult3D;
		}

		if (rayStart.z < minMaxZ.m_min && fwdNormal.z > 0.f)
		{
			Vec3& impactPos = raycastResult3D.m_impactPos;
			scaleFactor = (minMaxZ.m_min - rayStart.z) / fwdNormal.z;
			
			impactPos.x = rayStart.x + (fwdNormal.x * scaleFactor);
			impactPos.y = rayStart.y + (fwdNormal.y * scaleFactor);
			impactPos.z = minMaxZ.m_min;

			if (IsPointInsideDisc2D(Vec2(impactPos.x, impactPos.y), centerXY, radius))
			{
				raycastResult3D.m_didImpact = true;
				raycastResult3D.m_impactDist = (impactPos - rayStart).GetLength();
				raycastResult3D.m_impactNormal = Vec3(0.f, 0.f, -1.f);

				return raycastResult3D;
			}
		}

		if (rayStart.z > minMaxZ.m_max && fwdNormal.z < 0.f)
		{
			Vec3& impactPos = raycastResult3D.m_impactPos;
			scaleFactor = (minMaxZ.m_max - rayStart.z) / fwdNormal.z;

			impactPos.x = rayStart.x + (fwdNormal.x * scaleFactor);
			impactPos.y = rayStart.y + (fwdNormal.y * scaleFactor);
			impactPos.z = minMaxZ.m_max;

			if (IsPointInsideDisc2D(Vec2(impactPos.x, impactPos.y), centerXY, radius))
			{
				raycastResult3D.m_didImpact = true;
				raycastResult3D.m_impactDist = (impactPos - rayStart).GetLength();
				raycastResult3D.m_impactNormal = Vec3(0.f, 0.f, 1.f);

				return raycastResult3D;
			}
		}
	}

	return raycastResult3D;
}
