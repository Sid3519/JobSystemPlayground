#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"

//--------------------------------------------------------------------------------------------------
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"

//--------------------------------------------------------------------------------------------------
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern RandomNumberGenerator* g_rng;
// extern AudioSystem* g_audio;

//--------------------------------------------------------------------------------------------------
Game::Game()
{
}

//--------------------------------------------------------------------------------------------------
Game::~Game()
{
}

//--------------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_worldCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(MAP_SIZE_X, MAP_SIZE_Y));
	SetDesiredState(GAME_STATE_ATTRACT);

	InitializeTiles();
	CreateTestJobs();
}

//--------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
}

//--------------------------------------------------------------------------------------------------
void Game::KeyboardUpdate()
{
	XboxController const& controllerButton = g_theInput->GetController(0);
	if (g_theInput->IsKeyDown('N') && !g_theInput->IsKeyDown_WasDown('N') ||
		controllerButton.WasButtonJustPressed(XboxButtonID::START))
	{
	}

	if (g_theInput->IsKeyDown('I') && !g_theInput->IsKeyDown_WasDown('I'))
	{
	}

	if (g_theInput->IsKeyDown(' ') && !g_theInput->IsKeyDown_WasDown(' ') ||
		controllerButton.WasButtonJustPressed(XboxButtonID::A_BUTTON))
	{
	}
}

//--------------------------------------------------------------------------------------------------
void Game::Update()
{
	UpdateGameState();

	KeyboardUpdate();
	UpdateTileStatus();
}

//--------------------------------------------------------------------------------------------------
void Game::Render() const
{
	RenderGameState();
}


//--------------------------------------------------------------------------------------------------
void Game::UpdateTestJobs()
{
	Job* retrievedJob = nullptr;
	for ( ;; )
	{
		retrievedJob = g_theJobSystem->RetrieveCompletedJob();
		if (retrievedJob)
		{

		}
		else
		{
			break;
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::UpdateTileStatus()
{
	for (int tileIndex = 0; tileIndex < NUM_OF_TILES; ++tileIndex)
	{
		Tile&		currentTile		= m_tiles[tileIndex];
		TestJob*	currentTestJob	= m_testJobs[tileIndex];

		currentTile.m_tileStatus = currentTestJob->m_status;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::RenderTiles() const
{
	static std::vector<Vertex_PCU> tempVerts;
	constexpr int VERTS_PER_QUAD = 4;
	tempVerts.reserve(NUM_OF_TILES * VERTS_PER_QUAD);
	
	static std::vector<unsigned int> tempIndexes;
	constexpr int INDEXES_PER_QUAD = 6;
	tempVerts.reserve(NUM_OF_TILES * INDEXES_PER_QUAD);

	for (int tileIndex = 0; tileIndex < NUM_OF_TILES; ++tileIndex)
	{
		Tile const& currentTile = m_tiles[tileIndex];
		Rgba8 tileColor(Rgba8::WHITE);
		switch (currentTile.m_tileStatus)
		{
		case JOB_STATUS_QUEUED:
		{
			tileColor = Rgba8::RED;
			break;
		}
		case JOB_STATUS_CLAIMED_AND_EXECUTING:
		{
			tileColor = Rgba8::YELLOW;
			break;
		}
		case JOB_STATUS_COMPLETED:
		{
			tileColor = Rgba8::GREEN;
			break;
		}
		case JOB_STATUS_RETRIEVED_AND_RETIRED:
		{
			tileColor = Rgba8::BLUE;
			break;
		}
		default:
			break;
		}
		AddVertsForAABB2D(tempVerts, tempIndexes, currentTile.m_bounds, tileColor);
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawIndexedArray((int)tempVerts.size(), tempVerts.data(), (int)tempIndexes.size(), tempIndexes.data());

	tempVerts.clear();
	tempIndexes.clear();
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeTiles()
{
	for (int tileIndex = 0; tileIndex < NUM_OF_TILES; ++tileIndex)
	{
		Tile& currentTile = m_tiles[tileIndex];
		Vec2 currentTileCoords = GetTileCoordsFromTileIndex(tileIndex);
		AABB2 cosmeticTileBounds(currentTileCoords.x, currentTileCoords.y, currentTileCoords.x + 1.f, currentTileCoords.y + 1.f);
		cosmeticTileBounds.AddPadding(-0.05f, -0.05f);
		currentTile.m_bounds = cosmeticTileBounds;
	}
}


//--------------------------------------------------------------------------------------------------
Vec2 Game::GetTileCoordsFromTileIndex(int tileIndex) const
{
	Vec2 tileCoords;
	tileCoords.x = float(tileIndex % MAP_SIZE_X);
	tileCoords.y = float(tileIndex / MAP_SIZE_X);
	return tileCoords;
}


//--------------------------------------------------------------------------------------------------
void Game::CreateTestJobs()
{
	for (int tileIndex = 0; tileIndex < NUM_OF_TILES; ++tileIndex)
	{
		Vec2 tileCoords = GetTileCoordsFromTileIndex(tileIndex);
		int sleepMS		= g_rng->RollRandomIntInRange(50, 3000);
		TestJob* job	= new TestJob(int(tileCoords.x), (int)tileCoords.y, sleepMS);
		g_theJobSystem->QueueNewJob(job);
		m_testJobs[tileIndex] = job;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::EnterState(GameState gameState)
{
	switch (gameState)
	{
	case GAME_STATE_ATTRACT:
	{
		EnterAttract();
		break;
	}
	case GAME_STATE_PLAYING:
	{
		EnterPlaying();
		break;
	}
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------
void Game::ExitState(GameState gameState)
{
	switch (gameState)
	{
	case GAME_STATE_ATTRACT:
	{
		ExitAttract();
		break;
	}
	case GAME_STATE_PLAYING:
	{
		ExitPlaying();
		break;
	}
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------
void Game::EnterAttract()
{
}

//--------------------------------------------------------------------------------------------------
void Game::EnterPlaying()
{
}

//--------------------------------------------------------------------------------------------------
void Game::ExitAttract()
{
}

//--------------------------------------------------------------------------------------------------
void Game::ExitPlaying()
{
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateAttract()
{
}

//--------------------------------------------------------------------------------------------------
void Game::UpdatePlaying()
{
	UpdateTestJobs();
}

//--------------------------------------------------------------------------------------------------
void Game::RenderAttract() const
{
	g_theRenderer->ClearScreen(Rgba8::CRIMSON);
}

//--------------------------------------------------------------------------------------------------
void Game::RenderPlaying() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	
	g_theRenderer->BeginCamera(m_worldCamera);
	RenderTiles();
	g_theRenderer->EndCamera(m_worldCamera);
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateGameState()
{
	if (m_currentState != m_desiredState)
	{
		ExitState(m_currentState);
		m_currentState = m_desiredState;
		EnterState(m_currentState);
	}

	switch (m_currentState)
	{
	case GAME_STATE_ATTRACT:
	{
		UpdateAttract();
		break;
	}
	case GAME_STATE_PLAYING:
	{
		UpdatePlaying();
		break;
	}
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderGameState() const
{
	switch (m_currentState)
	{
	case GAME_STATE_ATTRACT:
	{
		RenderAttract();
		break;
	}
	case GAME_STATE_PLAYING:
	{
		RenderPlaying();
		break;
	}
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------
GameState Game::GetCurrentState() const
{
	return m_currentState;
}

//--------------------------------------------------------------------------------------------------
void Game::SetDesiredState(GameState desiredState)
{
	m_desiredState = desiredState;
}


//--------------------------------------------------------------------------------------------------
void TestJob::Execute()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepMS));
}
