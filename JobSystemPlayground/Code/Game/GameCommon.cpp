#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

void DebugDrawLine(Vec2 const& startPoint, Vec2 const& endPoint, float thickness, Rgba8 const& color)
{
	Vec2 startToEndVec = endPoint - startPoint;

	startToEndVec.Normalize();
	float halfThickness = thickness * 0.5f;
	Vec2 forwardDir = startToEndVec * halfThickness;
	Vec2 leftDir = forwardDir.GetRotated90Degrees();
	
	Vec3 bottomLeft { startPoint - forwardDir + leftDir, 0.f };
	Vec3 bottomRight { startPoint - forwardDir - leftDir, 0.f };
	Vec3 topLeft { endPoint + forwardDir + leftDir, 0.f };
	Vec3 topRight { endPoint + forwardDir - leftDir, 0.f };

	Vertex_PCU lineVerts[] 
	{
		Vertex_PCU {bottomLeft, color}, 
		Vertex_PCU {bottomRight, color}, 
		Vertex_PCU {topRight, color},

		Vertex_PCU {topRight, color}, 
		Vertex_PCU {topLeft, color}, 
		Vertex_PCU {bottomLeft, color}
	};

	constexpr int NUM_VERTS = sizeof(lineVerts) / sizeof(Vertex_PCU);
	
	g_theRenderer->DrawVertexArray(NUM_VERTS, lineVerts);
}

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;

	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;

	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;

	float startDegrees = 0;
	float endDegrees = startDegrees + DEGREES_PER_SIDE;

	Vertex_PCU vertexes[NUM_VERTS] = {};

	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		Vec3 innerLocalStartPos = MakeFromPolarDegrees(startDegrees, innerRadius);
		Vec3 outerLocalStartPos = MakeFromPolarDegrees(startDegrees, outerRadius);
		Vec3 outerLocalEndPos = MakeFromPolarDegrees(endDegrees, outerRadius);
		Vec3 innerLocalEndPos = MakeFromPolarDegrees(endDegrees, innerRadius);

		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;

		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertINdexF = (6 * sideNum) + 5;
		
		Vec3 pos[]{
			Vec3(center.x + innerLocalStartPos.x, center.y + innerLocalStartPos.y),
			Vec3(center.x + outerLocalStartPos.x, center.y + outerLocalStartPos.y),
			Vec3(center.x + outerLocalEndPos.x, center.y + outerLocalEndPos.y),
			Vec3(center.x + innerLocalEndPos.x, center.y + innerLocalEndPos.y)
		};

		vertexes[vertIndexA].m_position = pos[INNER_END_POS];
		vertexes[vertIndexB].m_position = pos[INNER_START_POS];
		vertexes[vertIndexC].m_position = pos[OUTER_START_POS];

		vertexes[vertIndexA].m_color = color;
		vertexes[vertIndexB].m_color = color;
		vertexes[vertIndexC].m_color = color;


		vertexes[vertIndexD].m_position = pos[INNER_END_POS];
		vertexes[vertIndexE].m_position = pos[OUTER_START_POS];
		vertexes[vertINdexF].m_position = pos[OUTER_END_POS];

		vertexes[vertIndexD].m_color = color;
		vertexes[vertIndexE].m_color = color;
		vertexes[vertINdexF].m_color = color;

		startDegrees = endDegrees;
		endDegrees += DEGREES_PER_SIDE;
	}
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS, vertexes);
}
