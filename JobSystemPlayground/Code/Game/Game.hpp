#pragma once

//--------------------------------------------------------------------------------------------------
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"


//--------------------------------------------------------------------------------------------------
constexpr int MAP_SIZE_X	= 40;
constexpr int MAP_SIZE_Y	= 20;
constexpr int NUM_OF_TILES	= MAP_SIZE_X * MAP_SIZE_Y;


//--------------------------------------------------------------------------------------------------
class TestJob : public Job
{
public:
	TestJob(int x, int y, int sleepMS) :
		m_tileCoords(x, y),
		m_sleepMS(sleepMS)
	{};
	virtual void Execute() override;

	IntVec2 m_tileCoords;
	int		m_sleepMS = 0;
};


//--------------------------------------------------------------------------------------------------
enum GameState
{
	GAME_STATE_INVALID = -1,

	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,

	GAME_STATE_COUNT,
};


//--------------------------------------------------------------------------------------------------
struct Tile
{
	AABB2		m_bounds;
	JobStatus	m_tileStatus = JOB_STATUS_INVALID;
};


//--------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void Shutdown();
	void KeyboardUpdate();
	void Update();
	void Render() const;
	
	void	UpdateTestJobs();
	void	UpdateTileStatus();
	void	RenderTiles() const;
	void	InitializeTiles();
	Vec2	GetTileCoordsFromTileIndex(int tileIndex) const;
	void	CreateTestJobs();

private:
	void EnterState(GameState gameState);
	void ExitState(GameState gameState);

	void EnterAttract();
	void EnterPlaying();
	
	void ExitAttract();
	void ExitPlaying();

	void UpdateAttract();
	void UpdatePlaying();

	void RenderAttract() const;
	void RenderPlaying() const;

	void UpdateGameState();
	void RenderGameState() const;
public:
	GameState	GetCurrentState() const;
	void		SetDesiredState(GameState desiredState);

private:
	Camera		m_worldCamera	= {};
	Tile		m_tiles[NUM_OF_TILES];
	TestJob*	m_testJobs[NUM_OF_TILES];
	GameState	m_currentState	= GAME_STATE_INVALID;
	GameState	m_desiredState	= GAME_STATE_INVALID;
};