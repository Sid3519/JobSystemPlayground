#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/EngineBuildPreferences.hpp"

#include <vector>

#define DX_SAFE_RELEASE(dxObject) { if ((dxObject) != nullptr) { (dxObject)->Release(); (dxObject) = nullptr; } }

#if defined(OPAQUE)
#undef OPAQUE
#endif

//--------------------------------------------------------------------------------------------------
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11BlendState;
struct ID3D11SamplerState;

//--------------------------------------------------------------------------------------------------
class Window;
class Texture;
class BitmapFont;
class Shader;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

//--------------------------------------------------------------------------------------------------
enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE,
	COUNT,
};

//--------------------------------------------------------------------------------------------------
enum class SamplerMode
{
	POINT_CLAMP,				
	BILINEAR_WRAP,						// Averaging 
	COUNT,
};

//--------------------------------------------------------------------------------------------------
enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT,	
};

//--------------------------------------------------------------------------------------------------
enum class DepthMode
{
	DISABLED,
	ENABLED,
	COUNT,
};

//--------------------------------------------------------------------------------------------------
struct RendererConfig
{
	Window* m_window = nullptr;
};

//--------------------------------------------------------------------------------------------------
enum LightType
{
	LIGHT_TYPE_INVALID = -1,

	LIGHT_TYPE_POINT_LIGHT,
	LGIHT_TYPE_SPOT_LIGHT,

	LIGHT_TYPE_COUNT,
};
//------------------------------------------------------------------------------------------------
struct Light
{
	bool		Enabled = false;						 // 4  bytes
	Vec3		Position = Vec3::ZERO;					 // 12 bytes
	//-----------------------------------				(16 byte boundary)
	Vec3		Direction = Vec3::ZERO;					 // 12 bytes
	int         LightType = LIGHT_TYPE_POINT_LIGHT;      // 4  bytes
	//-----------------------------------				(16 byte boundary)
	float		Color[4];								 // 16 bytes
	//-----------------------------------				(16 byte boundary)
	float       SpotAngle;								 // 4 bytes
	float       ConstantAttenuation;					 // 4 bytes
	float       LinearAttenuation;						 // 4 bytes
	float       QuadraticAttenuation;					 // 4 bytes
	//-----------------------------------				(16 byte boundary)
};

//--------------------------------------------------------------------------------------------------
constexpr int MAX_LIGHTS = 8;
//--------------------------------------------------------------------------------------------------
class Renderer
{
public:
	Renderer(RendererConfig const& config);
	~Renderer();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(Rgba8 const& clearColor);
	void BeginCamera(Camera const& camera);
	void EndCamera(Camera const& camera);
	void DrawVertexArray(int numVertexes, Vertex_PCU const* vertexes);
	void DrawVertexArray(std::vector<Vertex_PCU> const& vertexes);
	void DrawIndexedArray(int numVertexes, Vertex_PCU const* vertexes, int numIndexes, unsigned int const* indexes);
	void DrawIndexedArray(int numVertexes, Vertex_PNCU const* vertexes, int numIndexes, unsigned int const* indexes);
	RendererConfig const& GetConfig() const { return m_config;  }

	Texture* CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, unsigned char* texelData);
	void BindTexture(Texture const* texture);
	Texture*		CreateOrGetTextureFromFile(char const* imageFilePath);
	BitmapFont*		CreateOrGetBitmapFont(char const* bitmapFontFilePathWithNoExtension);
	Shader*			CreateOrGetShader(char const* shaderFilePath);

	Texture*	GetTextureForFileName	(char const* imageFilePath);
	BitmapFont* GetBitmapFontForFileName(char const* bitmapFontFilePathWithNoExtension);

	void BindShader(Shader* shader);
	Shader* CreateShader(char const* shaderName, char const* shaderSource);
	Shader* CreateShader(char const* shaderName);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);

	VertexBuffer* CreateVertexBuffer(size_t const size);
	VertexBuffer* CreateVertexBuffer(size_t const size, unsigned int stride);
	IndexBuffer* CreateIndexBuffer(size_t const size);
	void CopyCPUToGPU(void const* data, size_t size, VertexBuffer*& vbo);
	void CopyCPUToGPU(void const* data, size_t size, IndexBuffer*& ibo);
	void BindVertexBuffer(VertexBuffer* vbo);
	void BindIndexBuffer(IndexBuffer* ibo);

	ConstantBuffer* CreateConstantBuffer(size_t const size);
	void CopyCPUToGPU(void const* data, size_t size, ConstantBuffer*& cbo);
	void BindConstantBuffer(int slot, ConstantBuffer* cbo);

	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0);
	void DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0, int vertexOffset = 0);
	void DrawIndexed(int indexCount, int indexOffset = 0, int vertexOffset = 0);

	void CreateAndInitializeBlendModes();
	void CreateSamplerModes();
	void CreateRasterizerMode();
	void CreateDepthMode();
	void SetBlendMode(BlendMode blendMode);
	void SetSamplerMode(SamplerMode samplerMode);
	void SetRasterizerMode(RasterizerMode rasterizerMode);
	void SetDepthMode(DepthMode depthMode);
	void SetStatesIfChanged();
	void SetModelConstants(Mat44 const& modelMatrix = Mat44(), Rgba8 const& modelColor = Rgba8::WHITE);
	void SetLightingConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity);
	void SetLightAt(Light const& lightToSet, int indexToSet);

private:
	Texture*		CreateTextureFromFile	(char const* imageFilePath);
	Texture*		CreateTextureFromImage	(Image const& image);
	BitmapFont*		CreateBitmapFont		(char const* imageFilePathWithNoExtension);

protected:
	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;	// Object that holds all the information about the render target. In this case it's the back-buffer
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;
	ID3D11Device* m_d3d11Device = nullptr;	// Virtual adapter used to create resources
	ID3D11DeviceContext* m_d3d11DeviceContext = nullptr;	// contains information about the drawing capabilities of a device, generates rendering commands
	IDXGISwapChain* m_swapChain = nullptr;	// implements one of more buffers to store rendered data before presenting it to the output
	
	ID3D11BlendState* m_blendState = nullptr;
	ID3D11SamplerState* m_d3d11SamplerState = nullptr;
	ID3D11RasterizerState* m_d3d11RasterizeState = nullptr;

	BlendMode m_desiredBlendMode = BlendMode::ALPHA;
	SamplerMode m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	RasterizerMode m_desiredRasterizedMode = RasterizerMode::SOLID_CULL_BACK;
	DepthMode m_desiredDepthMode = DepthMode::ENABLED;

	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = {};
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};
	ID3D11DepthStencilState* m_depthStencilStates[(int)(DepthMode::COUNT)] = {};

	
	std::vector<Shader*> m_loadedShaders;
	Shader const* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;
	
	VertexBuffer* m_immediateVBO = nullptr;
	VertexBuffer* m_immediatePNCUVBO = nullptr;
	IndexBuffer* m_immediateIBO = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightingCBO = nullptr;

	RendererConfig m_config;

	Texture const* m_defaultTexture = nullptr;
	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;
private:
	// Should this be a pointer
	Light m_lights[MAX_LIGHTS];
};