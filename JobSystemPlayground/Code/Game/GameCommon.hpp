#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"

constexpr int NUM_TRIANGLE_VERTS = 3;
constexpr float TRI_LENGTH = 20.f;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X * 0.5f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y * 0.5f;

constexpr float ATTRACT_MODE_SIZE_X	= 1600.f;
constexpr float ATTRACT_MODE_SIZE_Y	= 800.f;
constexpr float ATTRACT_MODE_CENTER_X	= ATTRACT_MODE_SIZE_X * 0.5f;
constexpr float ATTRACT_MODE_CENTER_Y	= ATTRACT_MODE_SIZE_Y * 0.5f;

enum
{
	INNER_START_POS,
	OUTER_START_POS,
	OUTER_END_POS,
	INNER_END_POS
};

void DebugDrawLine(Vec2 const& startPoint, Vec2 const& endPoint, float thickness, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);