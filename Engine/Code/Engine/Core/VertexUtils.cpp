#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"


//--------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, scaleXY, rotationDegreesAboutZ, translationXY);
	}
}


//--------------------------------------------------------------------------------------------------
void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, Mat44 transform)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		pos = transform.TransformPosition3D(pos);
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	Vec2 dispFromBoneStartToBoneEnd = boneEnd - boneStart;
	Vec2 dispFromBoneStartToBoneEndRotated90Degrees = dispFromBoneStartToBoneEnd.GetRotated90Degrees();
	Vec2 dispFromBoneStartToBoneEndRotated90DegreesClamped = dispFromBoneStartToBoneEndRotated90Degrees.GetClamped(radius);


	Vec2 BR = boneStart - dispFromBoneStartToBoneEndRotated90DegreesClamped; // BottomRight
	Vec2 TR = boneEnd - dispFromBoneStartToBoneEndRotated90DegreesClamped;
	Vec2 TL = boneEnd + dispFromBoneStartToBoneEndRotated90DegreesClamped;
	Vec2 BL = boneStart + dispFromBoneStartToBoneEndRotated90DegreesClamped; // BottomLeft

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TR.x, TR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color));

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color));
	verts.push_back(Vertex_PCU(Vec3(BL.x, BL.y), color));

	int numOfTriangles = 18;
	constexpr int vertsPerTriangle = 3;
	float degreesPerSide = 180.f / (float)numOfTriangles;

	float currentOrientation = (TR - boneEnd).GetOrientationDegrees();


	for (int sideNum = 0; sideNum < numOfTriangles; ++sideNum)
	{
		Vec3 sectorTip = Vec3(boneEnd.x, boneEnd.y);
		Vec3 vertex1 = sectorTip + MakeFromPolarDegrees(currentOrientation, radius);
		Vec3 vertex2 = sectorTip + MakeFromPolarDegrees(currentOrientation + degreesPerSide, radius);

		verts.push_back(Vertex_PCU(sectorTip, color));
		verts.push_back(Vertex_PCU(vertex1, color));
		verts.push_back(Vertex_PCU(vertex2, color));

		currentOrientation = currentOrientation + degreesPerSide;
	}

	currentOrientation = (BL - boneStart).GetOrientationDegrees();

	for (int sideNum = 0; sideNum < numOfTriangles; ++sideNum)
	{
		Vec3 sectorTip = Vec3(boneStart.x, boneStart.y);
		Vec3 vertex1 = sectorTip + MakeFromPolarDegrees(currentOrientation, radius);
		Vec3 vertex2 = sectorTip + MakeFromPolarDegrees(currentOrientation + degreesPerSide, radius);

		verts.push_back(Vertex_PCU(sectorTip, color));
		verts.push_back(Vertex_PCU(vertex1, color));
		verts.push_back(Vertex_PCU(vertex2, color));

		currentOrientation = currentOrientation + degreesPerSide;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color)
{
	int numOfTriangles = 32;
	float degreesPerSide = 360.f / (float)numOfTriangles;
	float currentOrientation = 0.f;

	for (int sideNum = 0; sideNum < numOfTriangles; ++sideNum)
	{
		Vec3 sectorTip = Vec3(center.x, center.y);
		Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
		Vec3 localVertex1 = MakeFromPolarDegrees(currentOrientation, radius);
		Vec3 vertex1 = sectorTip + localVertex1;
		float vertex1U = RangeMap(localVertex1.x, -radius, radius, 0.f, 1.f);
		float vertex1V = RangeMap(localVertex1.y, -radius, radius, 0.f, 1.f);
		Vec3 localVertex2 = MakeFromPolarDegrees(currentOrientation + degreesPerSide, radius);
		Vec3 vertex2 = sectorTip + localVertex2;
		float vertex2U = RangeMap(localVertex2.x, -radius, radius, 0.f, 1.f);
		float vertex2V = RangeMap(localVertex2.y, -radius, radius, 0.f, 1.f);

		verts.push_back(Vertex_PCU(sectorTip, color, sectorTipUV));
		verts.push_back(Vertex_PCU(vertex1, color, Vec2(vertex1U, vertex1V)));
		verts.push_back(Vertex_PCU(vertex2, color, Vec2(vertex2U, vertex2V)));

		currentOrientation = currentOrientation + degreesPerSide;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	int numOfQuads = 32;
	float degreesPerSide = 360.f / (float)numOfQuads;
	float currentOrientation = 0.f;
	float halfThickness = thickness * 0.5f;

	Vec3 sectorTip = Vec3(center.x, center.y);

	Vec3 currentVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 currentVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	currentOrientation += degreesPerSide;
	Vec3 nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 nextVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	AddVertsForQuad3D(verts, indexes, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

	currentOrientation += degreesPerSide;
	int currentIndex = (int)verts.size();
	for (int quadNum = 1; quadNum < numOfQuads; ++quadNum)
	{
		nextVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
		nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
		verts.push_back(Vertex_PCU(nextVertexEnd, color, Vec2()));
		verts.push_back(Vertex_PCU(nextVertexStart, color, Vec2()));

		indexes.push_back(currentIndex - 1);
		indexes.push_back(currentIndex - 2);
		indexes.push_back(currentIndex);

		indexes.push_back(currentIndex - 1);
		indexes.push_back(currentIndex);
		indexes.push_back(currentIndex + 1);

		currentOrientation += degreesPerSide;
		currentIndex += 2;
	}
}


//--------------------------------------------------------------------------------------------------
//#ToDo: Optimize
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	int numOfQuads = 32;
	float degreesPerSide = 360.f / (float)numOfQuads;
	float currentOrientation = 0.f;
	float halfThickness = thickness * 0.5f;

	Vec3 sectorTip = Vec3(center.x, center.y);

	Vec3 currentVertexStart	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 currentVertexEnd	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	currentOrientation		+= degreesPerSide;
	Vec3 nextVertexStart	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 nextVertexEnd		= sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	AddVertsForQuad3D(verts, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

	currentVertexStart	= nextVertexStart;
	currentVertexEnd	= nextVertexEnd;
	currentOrientation += degreesPerSide;

	for (int quadNum = 1; quadNum < numOfQuads; ++quadNum)
	{
		nextVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
		nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
		AddVertsForQuad3D(verts, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

		currentVertexStart	= nextVertexStart;
		currentVertexEnd	= nextVertexEnd;
		currentOrientation += degreesPerSide;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color, AABB2 const& UVs, int numLatitudeSlices)
{
	int numLongitudeSlices = 2 * numLatitudeSlices;

	float longitudeSliceAngle = 360.f / numLongitudeSlices;
	float latitudeSliceAngle = 180.f / numLatitudeSlices;
	float currentLongitudeAngle = 0.f;
	float currentLatitudeAngle = -90.f;

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	Vec2 uvBL;
	Vec2 uvBR;
	Vec2 uvTR;
	Vec2 uvTL;

	for (int longitudeSlice = 0; longitudeSlice < numLongitudeSlices; ++longitudeSlice)
	{
		currentLatitudeAngle = -90.0f;
		for (int latitudeSlice = 0; latitudeSlice < numLatitudeSlices; ++latitudeSlice)
		{
			TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle, currentLongitudeAngle, radius);
			TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle, currentLongitudeAngle + (longitudeSliceAngle), radius);
			BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle + (latitudeSliceAngle), currentLongitudeAngle + (longitudeSliceAngle), radius);
			BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle + (latitudeSliceAngle), currentLongitudeAngle, radius);

			uvBL.x = RangeMapClamped(currentLongitudeAngle, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvBL.y = RangeMapClamped(currentLatitudeAngle + latitudeSliceAngle, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			uvTR.x = RangeMapClamped(currentLongitudeAngle + longitudeSliceAngle, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvTR.y = RangeMapClamped(currentLatitudeAngle, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			AddVertsForQuad3D(verts, BL, BR, TR, TL, color, AABB2(uvBL.x, uvBL.y, uvTR.x, uvTR.y));

			currentLatitudeAngle += latitudeSliceAngle;
		}
		currentLongitudeAngle += longitudeSliceAngle;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& tint, AABB2 const& UVs)
{
	float degreesPerSlice = 360.f / numSlices;
	float degreesPerStack = 180.f / numStacks;
	float currentYawDegrees = 0.f;
	float currentPitchDegrees = -90.f;

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	Vec2 uvBL;
	Vec2 uvBR;
	Vec2 uvTR;
	Vec2 uvTL;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		currentPitchDegrees = -90.0f;
		for (int stackNum = 0; stackNum < numStacks; ++stackNum)
		{
			TL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius);
			TR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees + (degreesPerSlice), radius);
			BR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
			BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees, radius);

			uvBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvBL.y = RangeMapClamped(currentPitchDegrees + degreesPerStack, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			uvTR.x = RangeMapClamped(currentYawDegrees + degreesPerSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvTR.y = RangeMapClamped(currentPitchDegrees, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			AddVertsForQuad3D(verts, BL, BR, TR, TL, tint, AABB2(uvBL.x, uvBL.y, uvTR.x, uvTR.y));

			currentPitchDegrees += degreesPerStack;
		}
		currentYawDegrees += degreesPerSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForSphereZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& tint)
{
	float degreesPerSlice = 360.f / numSlices;
	float degreesPerStack = 180.f / numStacks;
	float currentYawDegrees = 0.f;
	float currentPitchDegrees = -90.f;
	int startIndex = (int)verts.size();

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	// // // // FIRST PASS: DRAWS THE TOP CONE SEGMENT OF THE SPHERE // // // //
	BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees, radius);
	BR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
	TL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius);

	verts.push_back(Vertex_PCU(BL, tint, Vec2()));
	verts.push_back(Vertex_PCU(BR, tint, Vec2()));
	verts.push_back(Vertex_PCU(TL, tint, Vec2()));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	int prevMiddle = startIndex + 1;
	int currentIndex = (int)verts.size();

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		currentPitchDegrees = -90.f;
		for (int stackNum = 0; stackNum < numStacks; ++stackNum)
		{
			if (stackNum == 0 && sliceNum == 0)
			{
				continue;
			}
			else
			{
				if (stackNum == 0)
				{
					BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
					verts.push_back(Vertex_PCU(BL, tint, Vec2()));
					indexes.push_back(prevMiddle);
					indexes.push_back(currentIndex);
					prevMiddle = currentIndex;
					indexes.push_back(startIndex + 2);
					currentIndex += 1;
					TL = BL;

				}
				else
				{

				}
			}
			currentPitchDegrees += degreesPerStack;
		}
		currentYawDegrees += degreesPerSlice;
	}
	// // // // END OF THE FIRST PASS // // // //

	// Vec3 northPole = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius); // Sphere Top, NorthPole
	// verts.push_back(Vertex_PCU(northPole, tint, Vec2()));
	// int startIndex = (int)verts.size();
	// 
	// for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	// {
	// 	currentPitchDegrees = -90.f;
	// 	for (int stackNum = 0; stackNum < numStacks; ++stackNum)
	// 	{
	// 		if (currentPitchDegrees = -90.f)
	// 		{
	// 			indexes.push_back(startIndex);
	// 		}
	// 	}
	// }

}


//--------------------------------------------------------------------------------------------------
void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, int numSlices, int numStacks, float lineThickness, Rgba8 const& color)
{
	float degreesPerSlice = 360.f / numSlices;
	float degreesPerStack = 180.f / numStacks;
	float currentYawDegrees = 0.f;
	float currentPitchDegrees = -90.f;

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	for (int longitudeSlice = 0; longitudeSlice < numSlices; ++longitudeSlice)
	{
		currentPitchDegrees = -90.0f;
		for (int latitudeSlice = 0; latitudeSlice < numStacks; ++latitudeSlice)
		{
			TL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius);
			TR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees + (degreesPerSlice), radius);
			BR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
			BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees, radius);

			AddVertsForLineSegment3D(verts, TL, TR, lineThickness, color);
			AddVertsForLineSegment3D(verts, TL, BL, lineThickness, color);

			currentPitchDegrees += degreesPerStack;
		}
		currentYawDegrees += degreesPerSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForUnitCylinderX3D(std::vector<Vertex_PCU>& verts, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	Vec3 sectorMinTip = Vec3(0.f, 0.f, 0.f);
	Vec3 sectorMaxTip = Vec3(1.f, 0.f, 0.f);

	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, 1.f);
		currentSectorVert1.z = currentSectorVert1.x;
		currentSectorVert1.x = 0.f;
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, 1.f);
		currentSectorVert2.z = currentSectorVert2.x;
		currentSectorVert2.x = 0.f;

		float currentSectorVert1U = RangeMap(currentSectorVert1.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		float currentSectorVert2U = RangeMap(currentSectorVert2.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMinVert2 = sectorMinTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));

		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;
		Vec3 currentSectorMaxVert2 = sectorMaxTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMaxVert2, tint, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert2UV));
		verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));

		Vec2 currentCylinderUVBL;
		currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVBL.y = UVs.m_mins.y;

		Vec2 currentCylinderUVTR;
		currentCylinderUVTR.x = RangeMapClamped(currentYawDegrees + degreesPerYawSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVTR.y = UVs.m_maxs.y;

		AddVertsForQuad3D(verts, currentSectorMinVert2, currentSectorMinVert1, currentSectorMaxVert1, currentSectorMaxVert2, tint, AABB2(currentCylinderUVBL, currentCylinderUVTR));
		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	Vec3 sectorMinTip = Vec3(centerXY, minMaxZ.m_min);
	Vec3 sectorMaxTip = Vec3(centerXY, minMaxZ.m_max);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		float currentSectorVert1U = RangeMap(currentSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
		float currentSectorVert2U = RangeMap(currentSectorVert2.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMinVert2 = sectorMinTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2UV.x, 1.f - currentSectorVert2UV.y)));
		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert1UV.x, 1.f - currentSectorVert1UV.y)));
		verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));

		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;
		Vec3 currentSectorMaxVert2 = sectorMaxTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMaxVert2, tint, currentSectorVert2UV));
		verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));

		Vec2 currentCylinderUVBL;
		currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVBL.y = UVs.m_mins.y;

		Vec2 currentCylinderUVTR;
		currentCylinderUVTR.x = RangeMapClamped(currentYawDegrees + degreesPerYawSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVTR.y = UVs.m_maxs.y;

		AddVertsForQuad3D(verts, currentSectorMinVert1, currentSectorMinVert2, currentSectorMaxVert2, currentSectorMaxVert1, tint, AABB2(currentCylinderUVBL, currentCylinderUVTR));

		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	// // // MISC // // //
	Vec3 sectorMinTip = Vec3(centerXY, minMaxZ.m_min);
	Vec3 sectorMaxTip = Vec3(centerXY, minMaxZ.m_max);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	Vec3 prevSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
	float prevSectorVert1U = RangeMap(prevSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
	float prevSectorVert1V = RangeMap(prevSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
	Vec2 prevSectorVert1UV = Vec2(prevSectorVert1U, prevSectorVert1V);

	Vec3 prevSectorMinVert1 = sectorMinTip + prevSectorVert1;
	Vec3 prevSectorMaxVert1 = sectorMaxTip + prevSectorVert1;


	// // // CYLINDER BOTTOM DISC // // //
	int sectorMinTipIndex = (int)verts.size();
	verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));
	verts.push_back(Vertex_PCU(prevSectorMinVert1, tint, Vec2(prevSectorVert1UV.x, 1.f - prevSectorVert1UV.y)));

	// // // CYLINDER BODY // // // 
	Vec2 prevCylinderUVBL;
	prevCylinderUVBL.x = UVs.m_mins.x;
	prevCylinderUVBL.y = UVs.m_mins.y;

	verts.push_back(Vertex_PCU(prevSectorMinVert1, tint, prevCylinderUVBL));

	Vec2 prevCylinderUVTL;
	prevCylinderUVTL.x = UVs.m_mins.x;
	prevCylinderUVTL.y = UVs.m_maxs.y;

	verts.push_back(Vertex_PCU(prevSectorMaxVert1, tint, prevCylinderUVTL));

	// // // CYLINDER TOP DISC // // //
	verts.push_back(Vertex_PCU(prevSectorMaxVert1, tint, prevSectorVert1UV));
	int sectorMaxTipIndex = (int)verts.size();
	verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));
	// // //           // // // 

	currentYawDegrees += degreesPerYawSlice;
	int currentIndex = (int)verts.size();
	for (int sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		float currentSectorVert1U = RangeMap(currentSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;

		// // // CYLINDER BOTTOM DISC // // //
		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert1UV.x, 1.f - currentSectorVert1UV.y)));
		indexes.push_back(sectorMinTipIndex);
		indexes.push_back(currentIndex);
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		currentIndex += 1;

		// // // CYLINDER BODY // // //
		Vec2 currentCylinderUVBL;
		currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVBL.y = UVs.m_mins.y;

		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, currentCylinderUVBL));

		Vec2 currentCylinderUVTL;
		currentCylinderUVTL.x = currentCylinderUVBL.x;
		currentCylinderUVTL.y = UVs.m_maxs.y;

		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentCylinderUVTL));
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		indexes.push_back(currentIndex);
		indexes.push_back(currentIndex + 1);

		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		indexes.push_back(currentIndex + 1);
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 3);
		}
		else
		{
			indexes.push_back(currentIndex - 4);
		}
		currentIndex += 2;

		// // // CYLINDER TOP DISC // // //
		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert1UV));
		indexes.push_back(sectorMaxTipIndex);
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		indexes.push_back(currentIndex);
		currentIndex += 1;
		// // //           // // // 

		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint)
{
	Vec3 sectorMinTip = Vec3(centerXY, minMaxZ.m_min);
	Vec3 sectorMaxTip = Vec3(centerXY, minMaxZ.m_max);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMinVert2 = sectorMinTip + currentSectorVert2;

		AddVertsForLineSegment3D(verts, currentSectorMinVert1, currentSectorMinVert2, lineThickness, tint);

		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;
		Vec3 currentSectorMaxVert2 = sectorMaxTip + currentSectorVert2;

		AddVertsForLineSegment3D(verts, currentSectorMaxVert1, currentSectorMaxVert2, lineThickness, tint);


		AddVertsForLineSegment3D(verts, currentSectorMaxVert1, currentSectorMinVert1, lineThickness, tint);
		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, int numSlices, AABB2 const& UVs)
{
	Vec3 dispSE = (end - start);
	Vec3 iForward = dispSE.GetNormalized();

	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	else
	{
		jLeft.Normalize();
	}
	Vec3 kUp = CrossProduct3D(iForward, jLeft);

	Mat44 transformationMatrix;
	float depth = dispSE.GetLength();
	transformationMatrix.SetIJK3D(iForward * depth, jLeft * radius, kUp * radius);
	transformationMatrix.SetTranslation3D(start);

	AddVertsForUnitCylinderX3D(verts, (float)numSlices, tint, UVs);

	TransformVertexArray3D(verts, transformationMatrix);

	// transformationMatrix.SetTranslation3D(start);
	// AddVertsForCylinderZ3D(verts, Vec2(0.f, 0.f), FloatRange(0.f, height.GetLength()), radius, (float)numSlices, tint, UVs);
	// TransformVertexArray3D(verts, transformationMatrix);
	// 
	// 
	// // transformationMatrix.SetTranslation3D(start);
	// Mat44 minTransformedMat = transformationMatrix;
	// 
	// 
	// // std::vector<Vertex_PCU> verts;
	// AddVertsForCylinderZ3D(verts, Vec2(start.x, start.y), FloatRange(start.z, start.z + 20.f), radius, (float)numSlices, tint, UVs);
	// for (int vertIndex = 0; vertIndex < verts.size(); ++vertIndex)
	// {
	// 	verts[vertIndex].m_position = minTransformedMat.TransformPosition3D(verts[vertIndex].m_position);
	// }

	// transformationMatrix.SetTranslation3D(end);
	// Mat44 maxTransformedMat = transformationMatrix;
	// 
	// Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	// float currentYawDegrees = 0.f;
	// float degreesPerYawSlice = 360.f / numSlices;
	// 
	// for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	// {
	// 	Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
	// 	currentSectorVert1.z = currentSectorVert1.x;
	// 	currentSectorVert1.x = 0.f;
	// 	float currentSectorVert1U = RangeMap(currentSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
	// 	float currentSectorVert1V = RangeMap(currentSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
	// 	Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);
	// 	Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
	// 	currentSectorVert2.z = currentSectorVert2.x;
	// 	currentSectorVert2.x = 0.f;
	// 	float currentSectorVert2U = RangeMap(currentSectorVert2.z, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
	// 	float currentSectorVert2V = RangeMap(currentSectorVert2.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
	// 	Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);
	// 	
	// 	Vec3 currentSectorMinVert1 = transformationMatrix.TransformPosition3D(currentSectorVert1);
	// 	Vec3 currentSectorMinVert2 = transformationMatrix.TransformPosition3D(currentSectorVert2);
	// 
	// 	verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2UV.x, 1.f - currentSectorVert2UV.y)));
	// 	verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert1UV.x, 1.f - currentSectorVert1UV.y)));
	// 	verts.push_back(Vertex_PCU(start, tint, sectorTipUV));
	// 
	// 	Vec3 currentSectorMaxVert1 = translationMatrix.TransformPosition3D(currentSectorVert1);
	// 	Vec3 currentSectorMaxVert2 = translationMatrix.TransformPosition3D(currentSectorVert2);
	// 
	// 	verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert1UV));
	// 	verts.push_back(Vertex_PCU(currentSectorMaxVert2, tint, currentSectorVert2UV));
	// 	verts.push_back(Vertex_PCU(end, tint, sectorTipUV));
	// 
	// 	Vec2 currentCylinderUVBL;
	// 	currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
	// 	currentCylinderUVBL.y = UVs.m_mins.y;
	// 	// 
	// 	Vec2 currentCylinderUVTR;
	// 	currentCylinderUVTR.x = RangeMapClamped(currentYawDegrees + degreesPerYawSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
	// 	currentCylinderUVTR.y = UVs.m_maxs.y;
	// 
	// 	AddVertsForQuad3D(verts, currentSectorMinVert1, currentSectorMinVert2, currentSectorMaxVert2, currentSectorMaxVert1, tint, AABB2(currentCylinderUVBL, currentCylinderUVTR));
	// 
	// 	currentYawDegrees += degreesPerYawSlice;
	// }
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 BL = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0);
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0);
	Vec3 TR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0);
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0);

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(BL, color, uvBL));
	verts.push_back(Vertex_PCU(BR, color, uvBR));
	verts.push_back(Vertex_PCU(TR, color, uvTR));

	verts.push_back(Vertex_PCU(BL, color, uvBL));
	verts.push_back(Vertex_PCU(TR, color, uvTR));
	verts.push_back(Vertex_PCU(TL, color, uvTL));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 BL = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0);
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0);
	Vec3 TR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0);
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0);

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	int startIndex = (int)verts.size();
	verts.emplace_back(BL, color, uvBL);
	verts.emplace_back(BR, color, uvBR);
	verts.emplace_back(TR, color, uvTR);
	verts.emplace_back(TL, color, uvTL);

	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 1);
	indexes.emplace_back(startIndex + 2);
	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 2);
	indexes.emplace_back(startIndex + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);

	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForQuad3D(verts, ESB, ENB, ENT, EST, color, UVs); // EAST FACE +X
	AddVertsForQuad3D(verts, WNB, WSB, WST, WNT, color, UVs); // WEST FACE -X
	AddVertsForQuad3D(verts, ENB, WNB, WNT, ENT, color, UVs); // NORTH FACE +Y
	AddVertsForQuad3D(verts, WSB, ESB, EST, WST, color, UVs); // SOUTH FACE -Y
	AddVertsForQuad3D(verts, WST, EST, ENT, WNT, color, UVs); // TOP FACE +Z
	AddVertsForQuad3D(verts, WNB, ENB, ESB, WSB, color, UVs); // BOTTOM FACE -Z
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& color)
{
	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);

	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForLineSegment3D(verts, ESB, ENB, lineThickness, color);
	AddVertsForLineSegment3D(verts, ESB, EST, lineThickness, color);
	AddVertsForLineSegment3D(verts, ENB, ENT, lineThickness, color);
	AddVertsForLineSegment3D(verts, EST, ENT, lineThickness, color);

	AddVertsForLineSegment3D(verts, WSB, WNB, lineThickness, color);
	AddVertsForLineSegment3D(verts, WSB, WST, lineThickness, color);
	AddVertsForLineSegment3D(verts, WNB, WNT, lineThickness, color);
	AddVertsForLineSegment3D(verts, WST, WNT, lineThickness, color);

	AddVertsForLineSegment3D(verts, ESB, WSB, lineThickness, color);
	AddVertsForLineSegment3D(verts, ENB, WNB, lineThickness, color);
	AddVertsForLineSegment3D(verts, ENT, WNT, lineThickness, color);
	AddVertsForLineSegment3D(verts, EST, WST, lineThickness, color);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(bottomLeft, color, uvBL));
	verts.push_back(Vertex_PCU(bottomRight, color, uvBR));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));

	verts.push_back(Vertex_PCU(bottomLeft, color, uvBL));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));
	verts.push_back(Vertex_PCU(topLeft, color, uvTL));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int startIndex = (int)verts.size();
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(bottomLeft, color, uvBL));
	verts.push_back(Vertex_PCU(bottomRight, color, uvBR));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));
	verts.push_back(Vertex_PCU(topLeft, color, uvTL));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int startIndex = (int)verts.size();
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	Vec3 dispBLToBR = bottomRight - bottomLeft;
	Vec3 dispBLToTL = topLeft - bottomLeft;
	Vec3 dirBLBR = dispBLToBR.GetNormalized();
	Vec3 dirBLTL = dispBLToTL.GetNormalized();

	Vec3 normal = CrossProduct3D(dirBLBR, dirBLTL);
	verts.push_back(Vertex_PNCU(bottomLeft, normal, color, uvBL));
	verts.push_back(Vertex_PNCU(bottomRight, normal, color, uvBR));
	verts.push_back(Vertex_PNCU(topRight, normal, color, uvTR));
	verts.push_back(Vertex_PNCU(topLeft, normal, color, uvTL));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int startIndex = (int)verts.size();
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvBMP = Vec2(((UVs.m_maxs.x + UVs.m_mins.x) * 0.5f), UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	Vec2 uvTMP = Vec2(((UVs.m_maxs.x + UVs.m_mins.x) * 0.5f), UVs.m_maxs.y);

	Vec3 dispBLToBR = bottomRight - bottomLeft;
	Vec3 dispBLToTL = topLeft - bottomLeft;
	Vec3 dirBLBR = dispBLToBR;
	Vec3 dirBLTL = dispBLToTL;
	Vec3 normal = CrossProduct3D(dirBLBR, dirBLTL);
	normal.Normalize();

	Vec3 bottomMidPoint = (bottomRight + bottomLeft) * 0.5f;
	Vec3 topMidPoint = (topRight + topLeft) * 0.5f;

	Vec3 dispBMPToBR = bottomRight - bottomMidPoint;
	Vec3 tangentRight = dispBMPToBR.GetNormalized();
	Vec3 dispBMPToBL = bottomLeft - bottomMidPoint;
	Vec3 tangentLeft = dispBMPToBL.GetNormalized();

	verts.push_back(Vertex_PNCU(bottomLeft, tangentLeft, color, uvBL));
	verts.push_back(Vertex_PNCU(bottomMidPoint, normal, color, uvBMP));
	verts.push_back(Vertex_PNCU(bottomRight, tangentRight, color, uvBR));
	verts.push_back(Vertex_PNCU(topRight, tangentRight, color, uvTR));
	verts.push_back(Vertex_PNCU(topMidPoint, normal, color, uvTMP));
	verts.push_back(Vertex_PNCU(topLeft, tangentLeft, color, uvTL));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 4);
	indexes.push_back(startIndex + 5);

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 4);

	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 3);
	indexes.push_back(startIndex + 4);

	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	Vec2 jBasisNormal = box.m_iBasisNormal.GetRotated90Degrees();
	Vec2 scalediBasis = box.m_iBasisNormal * box.m_halfDimensions.x;
	Vec2 scaledjBasis = jBasisNormal * box.m_halfDimensions.y;

	Vec2 BR = box.m_center + scalediBasis - scaledjBasis; // BottomRight
	Vec2 TR = box.m_center + scalediBasis + scaledjBasis;
	Vec2 TL = box.m_center - scalediBasis + scaledjBasis;
	Vec2 BL = box.m_center - scalediBasis - scaledjBasis; // BottomLeft

	Vec2 uvBL = Vec2(0.f, 0.f);
	Vec2 uvBR = Vec2(1.f, 0.f);
	Vec2 uvTR = Vec2(1.f, 1.f);
	Vec2 uvTL = Vec2(0.f, 1.f);

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color, uvBR));
	verts.push_back(Vertex_PCU(Vec3(TR.x, TR.y), color, uvTR));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color, uvTL));

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color, uvBR));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color, uvTL));
	verts.push_back(Vertex_PCU(Vec3(BL.x, BL.y), color, uvBL));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 dispFromStartToEnd = end - start;
	Vec2 normalizedDispSE = dispFromStartToEnd.GetNormalized();
	Vec2 perpendicular = normalizedDispSE.GetRotated90Degrees();
	float halfThickness = thickness * 0.5f;
	Vec2 scaledDisp = normalizedDispSE * halfThickness;
	Vec2 scaledPerpendicular = perpendicular * halfThickness;

	Vec2 BR = start - scaledPerpendicular - scaledDisp;
	Vec2 TR = end - scaledPerpendicular + scaledDisp;
	Vec2 TL = end + scaledPerpendicular + scaledDisp;
	Vec2 BL = start + scaledPerpendicular - scaledDisp;

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TR.x, TR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color));

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color));
	verts.push_back(Vertex_PCU(Vec3(BL.x, BL.y), color));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& color)
{
	Vec3 iForward = (end - start).GetNormalized();
	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	jLeft.Normalize();
	Vec3 kUp = CrossProduct3D(iForward, jLeft).GetNormalized();

	float halfThickness = thickness * 0.5f;

	Vec3 scaledForward = iForward * halfThickness;
	Vec3 scaledPerpendicularY = jLeft * halfThickness;
	Vec3 scaledPerpendicularZ = kUp * halfThickness;

	Vec3 ESB = start - scaledForward - scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 ENB = end + scaledForward - scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 ENT = end + scaledForward - scaledPerpendicularY + scaledPerpendicularZ;
	Vec3 EST = start - scaledForward - scaledPerpendicularY + scaledPerpendicularZ;

	Vec3 WNB = end + scaledForward + scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 WSB = start - scaledForward + scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 WST = start - scaledForward + scaledPerpendicularY + scaledPerpendicularZ;
	Vec3 WNT = end + scaledForward + scaledPerpendicularY + scaledPerpendicularZ;

	AddVertsForQuad3D(verts, ESB, ENB, ENT, EST, color); // EAST FACE +X
	AddVertsForQuad3D(verts, WNB, WSB, WST, WNT, color); // WEST FACE -X
	AddVertsForQuad3D(verts, ENB, WNB, WNT, ENT, color); // NORTH FACE +Y
	AddVertsForQuad3D(verts, WSB, ESB, EST, WST, color); // SOUTH FACE -Y
	AddVertsForQuad3D(verts, WST, EST, ENT, WNT, color); // TOP FACE +Z
	AddVertsForQuad3D(verts, WNB, ENB, ESB, WSB, color); // BOTTOM FACE -Z
}


//--------------------------------------------------------------------------------------------------
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	Vec2 dispFromTailToTip = tipPos - tailPos;
	Vec2 directionFromTailToTip = dispFromTailToTip.GetNormalized();
	Vec2 dirFromTip = directionFromTailToTip.GetRotated90Degrees();
	Vec2 arrowBasePos = tipPos - directionFromTailToTip;
	float halfArrowSize = arrowSize * 0.5f;
	Vec2 scaledDirFromTip = dirFromTip * halfArrowSize;
	Vec2 arrowPosRight = arrowBasePos + scaledDirFromTip;
	Vec2 arrowPosLeft = arrowBasePos - scaledDirFromTip;

	AddVertsForLineSegment2D(verts, tailPos, tipPos, lineThickness, color);
	AddVertsForLineSegment2D(verts, tipPos, arrowPosRight, lineThickness, color);
	AddVertsForLineSegment2D(verts, tipPos, arrowPosLeft, lineThickness, color);
}


//--------------------------------------------------------------------------------------------------
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, Mat44 const& transform)
{
	TransformVertexArray3D((int)verts.size(), verts.data(), transform);
}


//--------------------------------------------------------------------------------------------------
AABB2 GetVertexBounds2D(std::vector<Vertex_PCU> const& verts)
{
	Vec2 firstPos;
	firstPos.x = verts[0].m_position.x;
	firstPos.y = verts[0].m_position.y;
	AABB2 vertexBounds = AABB2(firstPos, firstPos);

	for (int vertIndex = 1; vertIndex < verts.size(); ++vertIndex)
	{
		Vec3 const& currentVertPos = verts[vertIndex].m_position;
		if (vertexBounds.m_mins.x > currentVertPos.x)
		{
			vertexBounds.m_mins.x = currentVertPos.x;
		}
		if (vertexBounds.m_mins.y > currentVertPos.y)
		{
			vertexBounds.m_mins.y = currentVertPos.y;
		}
		if (vertexBounds.m_maxs.x < currentVertPos.x)
		{
			vertexBounds.m_maxs.x = currentVertPos.x;
		}
		if (vertexBounds.m_maxs.y < currentVertPos.y)
		{
			vertexBounds.m_maxs.y = currentVertPos.y;
		}
	}

	return vertexBounds;
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, int numSlices, AABB2 const& UVs)
{
	Vec3 dispSE = (end - start);
	Vec3 iForward = dispSE.GetNormalized();

	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	else
	{
		jLeft.Normalize();
	}
	Vec3 kUp = CrossProduct3D(iForward, jLeft);

	Mat44 transformationMatrix;
	transformationMatrix.SetIJK3D(iForward, jLeft, kUp);
	transformationMatrix.SetTranslation3D(start);

	Vec3 sectorMinTip = Vec3(0.f, 0.f, 0.f);
	sectorMinTip = transformationMatrix.TransformPosition3D(sectorMinTip);
	float depth = dispSE.GetLength();
	Vec3 sectorMaxTip = Vec3(depth, 0.f, 0.f);
	sectorMaxTip = transformationMatrix.TransformPosition3D(sectorMaxTip);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);

	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		currentSectorVert1.z = currentSectorVert1.x;
		currentSectorVert1.x = 0.f;
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
		currentSectorVert2.z = currentSectorVert2.x;
		currentSectorVert2.x = 0.f;

		float currentSectorVert1U = RangeMap(currentSectorVert1.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		float currentSectorVert2U = RangeMap(currentSectorVert2.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = currentSectorVert1;
		Vec3 currentSectorMinVert2 = currentSectorVert2;

		currentSectorMinVert1 = transformationMatrix.TransformPosition3D(currentSectorMinVert1);
		currentSectorMinVert2 = transformationMatrix.TransformPosition3D(currentSectorMinVert2);

		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));

		verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, currentSectorVert2UV));

		currentYawDegrees += degreesPerYawSlice;
	}
}