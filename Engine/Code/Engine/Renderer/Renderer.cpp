#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "ThirdParty/stb/stb_image.h"

#include "Engine/Window/Window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#if defined(ENGINE_DEBUG_RENDERER)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#if defined(OPAQUE)
#undef OPAQUE
#endif

struct LightingConstants
{
	Vec3		sunDirection;
	float		sunIntensity;
	float		ambientIntensity;
	Vec3		sunDirectionPadding;
	Light		Lights[MAX_LIGHTS];		// 16 * 8	bytes
};

static int const s_lightingConstantsSlot = 1;

struct CameraConstants
{
	Mat44 projectionMatrix;
	Mat44 viewMatrix;
};

static int const s_cameraConstantsSlot = 2;

struct ModelConstants
{
	Mat44 modelMatrix;
	float modelColor[4];
};

static int const s_modelConstantsSlot = 3;

Renderer::Renderer(RendererConfig const& config) :
	m_config(config)
{
}

Renderer::~Renderer()
{

}

void Renderer::Startup()
{
#if defined (ENGINE_DEBUG_RENDERER)
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll");
	}

	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module");
	}
#endif

	IntVec2 windowDim = m_config.m_window->GetClientDimensions(); // g_theWindow->GetClientDimensions();
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};										// swap chain is a collection of buffers

	swapChainDesc.BufferDesc.Width	=	windowDim.x;								// Back-buffer width
	swapChainDesc.BufferDesc.Height =	windowDim.y;								// Back-buffer height
	swapChainDesc.BufferDesc.Format =	DXGI_FORMAT_R8G8B8A8_UNORM;					// Color format 
	swapChainDesc.SampleDesc.Count	=	1;											// Multi sample anti aliasing (number of multi-samples)
	swapChainDesc.BufferUsage		=	DXGI_USAGE_RENDER_TARGET_OUTPUT;			// DXGI_USAGE_RENDER_TARGET_OUTPUT -> This specifies that the output should be used as the target output
	swapChainDesc.BufferCount		=	2;											// Number of buffers
	swapChainDesc.OutputWindow		=	(HWND)m_config.m_window->GetHwnd();			// The handle to the window
	swapChainDesc.Windowed			=	true;										// Specifies that the window is in windowed mode
	swapChainDesc.SwapEffect		=	DXGI_SWAP_EFFECT_FLIP_DISCARD;				// Flip signifies there is not copy between the buffers(back-buffer is swapped), Discards back-buffer after the swapchain

	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDERER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hResult;
	hResult = D3D11CreateDeviceAndSwapChain(	NULL,											// Let it be null unless you want to specify the video adapter
												D3D_DRIVER_TYPE_HARDWARE,						// Enables the use of D3D on the Hardware(GPU)
												NULL,											// Only useful when D3D_DRIVER_TYPE |^ is D3D_DRIVER_TYPE_SOFTWARE
												deviceFlags,
												NULL,											// Feature level related (d3d version AND feature list)
												NULL, 											// Feature level related (d3d version AND feature list)
												D3D11_SDK_VERSION,								// Lets the client know which version of DirectX was the game built on
												&swapChainDesc,									// Pointer to the Swap_chain_description struct
												&m_swapChain,
												&m_d3d11Device,
												NULL,											// Feature Level related
												&m_d3d11DeviceContext
											);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swap chain");
	}

	ID3D11Texture2D* backBuffer;
	// HRESULT is a data type that represents the completion status of a function
	hResult = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer); // Fills in the Back-buffer/ Gets the location of where in memory the back-buffer is
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not get swap chain buffer");
	}

	hResult = m_d3d11Device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView); // Create a COM object using that address to represent the render target
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could create render target view for swap chain buffer");
	}

	Shader* shader = CreateShader("Default", g_theShaderSource);
	// Shader* shader = CreateShader("Data/Shaders/Default");
	m_defaultShader = shader;
	BindShader(shader);

	Image defaultImage = Image(IntVec2(1, 1), Rgba8(255, 255, 255));
	defaultImage.m_imageFilePath = "DEFAULT";
	m_defaultTexture = CreateTextureFromImage(defaultImage);
	BindTexture(m_defaultTexture);

	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_immediatePNCUVBO = CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));
	m_immediateIBO = CreateIndexBuffer(sizeof(unsigned int));
	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_lightingCBO = CreateConstantBuffer(sizeof(LightingConstants));
	
	CreateAndInitializeBlendModes();
	CreateSamplerModes();

	backBuffer->Release();

	D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	depthTextureDesc.Width = windowDim.x;
	depthTextureDesc.Height = windowDim.y;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.SampleDesc.Count = 1;

	hResult = m_d3d11Device->CreateTexture2D(&depthTextureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil texture");
	}

	hResult = m_d3d11Device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilView);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil view");
	}

	CreateDepthMode();

	CreateRasterizerMode();

	SetModelConstants();
}

void Renderer::BeginFrame()
{
	m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

void Renderer::EndFrame()
{
	HRESULT hResult = m_swapChain->Present(0, 0);
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost application will now terminate");
	}
}

void Renderer::Shutdown()
{
	for (int samplerIndex = 0; samplerIndex < int(SamplerMode::COUNT); ++samplerIndex)
	{
		DX_SAFE_RELEASE(m_samplerStates[samplerIndex]);
	}
	for (int blendIndex = 0; blendIndex < int(BlendMode::COUNT); ++blendIndex)
	{
		DX_SAFE_RELEASE(m_blendStates[blendIndex]);
	}
	for (int rasterizerIndex = 0; rasterizerIndex < int(RasterizerMode::COUNT); ++rasterizerIndex)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[rasterizerIndex]);
	}
	for (int depthIndex = 0; depthIndex < int(DepthMode::COUNT); ++depthIndex)
	{
		DX_SAFE_RELEASE(m_depthStencilStates[depthIndex]);
	}

	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_d3d11DeviceContext);
	DX_SAFE_RELEASE(m_d3d11Device);
	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex)
	{
		delete m_loadedShaders[shaderIndex];
		m_loadedShaders[shaderIndex] = nullptr;
	}

	m_currentShader = nullptr;
	
	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); ++textureIndex)
	{
		delete m_loadedTextures[textureIndex];
		m_loadedTextures[textureIndex] = nullptr;
	}
	delete m_defaultTexture;
	m_defaultTexture = nullptr;

	delete m_immediateVBO;
	m_immediateVBO = nullptr;
	delete m_immediatePNCUVBO;
	m_immediatePNCUVBO = nullptr;
	delete m_immediateIBO;
	m_immediateIBO = nullptr;
	delete m_cameraCBO;
	m_cameraCBO = nullptr;
	delete m_modelCBO;
	m_modelCBO = nullptr;
	delete m_lightingCBO;
	m_lightingCBO = nullptr;

#if defined(ENGINE_DEBUG_RENDERER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}

void Renderer::ClearScreen(Rgba8 const& clearColor)
{
	float clearColorAsFloats[4] = {};
	clearColor.GetAsFloats(clearColorAsFloats);
	m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, clearColorAsFloats);
	m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void Renderer::BeginCamera(Camera const& camera)
{
	Mat44 projectionMat = camera.GetProjectionMatrix();
	Mat44 viewMat = camera.GetViewMatrix();
	CameraConstants camConstants = {};
	camConstants.projectionMatrix = projectionMat;
	camConstants.viewMatrix = viewMat;

	CopyCPUToGPU(&camConstants, sizeof(camConstants), m_cameraCBO);
	BindConstantBuffer(s_cameraConstantsSlot, m_cameraCBO);

	IntVec2 windowDim = m_config.m_window->GetClientDimensions();
	float width = (float)windowDim.x;
	float height = (float)windowDim.y;
	AABB2 clientBounds = AABB2(0.f, 0.f, width, height);

	D3D11_VIEWPORT viewport = {};
	AABB2 cameraViewport = camera.GetCameraViewport();
	
	AABB2 cameraBounds = clientBounds.GetBoxAtUVs(cameraViewport);
	Vec2 cameraDimensions = cameraBounds.GetDimensions();

	if (cameraViewport == AABB2::INVALID)
	{
		viewport.Width = width;
		viewport.Height = height;
	}
	else
	{
		viewport.TopLeftX = cameraBounds.m_mins.x;
		viewport.TopLeftY = height - cameraBounds.m_maxs.y;
		viewport.Width = cameraDimensions.x;
		viewport.Height = cameraDimensions.y;
	}

	viewport.MaxDepth = 1;

	m_d3d11DeviceContext->RSSetViewports(1, &viewport);
}

void Renderer::EndCamera(Camera const& camera)
{
	(void)camera;
}

void Renderer::DrawVertexArray(int numVertexes, Vertex_PCU const* vertexes)
{
	CopyCPUToGPU(vertexes, (size_t)m_immediateVBO->GetStride() * numVertexes, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexes);
}

void Renderer::DrawVertexArray(std::vector<Vertex_PCU> const& vertexes)
{
	int numVertexes = (int)vertexes.size();
	CopyCPUToGPU(vertexes.data(), (size_t)m_immediateVBO->GetStride() * numVertexes, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexes);
}

void Renderer::DrawIndexedArray(int numVertexes, Vertex_PCU const* vertexes, int numIndexes, unsigned int const* indexes)
{
	SetStatesIfChanged();
	CopyCPUToGPU(vertexes, (size_t)m_immediateVBO->GetStride() * numVertexes, m_immediateVBO);
	BindVertexBuffer(m_immediateVBO);
	CopyCPUToGPU(indexes, (size_t)m_immediateIBO->GetStride() * numIndexes, m_immediateIBO);
	BindIndexBuffer(m_immediateIBO);

	DrawIndexed(numIndexes);
}

void Renderer::DrawIndexedArray(int numVertexes, Vertex_PNCU const* vertexes, int numIndexes, unsigned int const* indexes)
{
	SetStatesIfChanged();
	CopyCPUToGPU(vertexes, (size_t)m_immediatePNCUVBO->GetStride() * numVertexes, m_immediatePNCUVBO);
	BindVertexBuffer(m_immediatePNCUVBO);
	CopyCPUToGPU(indexes, (size_t)m_immediateIBO->GetStride() * numIndexes, m_immediateIBO);
	BindIndexBuffer(m_immediateIBO);

	DrawIndexed(numIndexes);
}


Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, unsigned char* texelData)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	return newTexture;
}


void Renderer::BindTexture(Texture const* texture)
{
	if (texture == nullptr)
	{
		texture = m_defaultTexture;
	}
	m_d3d11DeviceContext->PSSetShaderResources(0, 1, &texture->m_shaderResourceView);
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	Image* newImage = new Image(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(*newImage);
	if (newTexture != nullptr)
	{
		m_loadedTextures.push_back(newTexture);
	}

	return newTexture;
}

Texture* Renderer::CreateTextureFromImage(Image const& image)
{
	IntVec2 textureDim = image.GetDimensions();
	D3D11_TEXTURE2D_DESC texture2DDesc = {};
	texture2DDesc.Width = textureDim.x;
	texture2DDesc.Height = textureDim.y;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture2DDesc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = image.GetRawData();
	if (texture2DDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		subresourceData.SysMemPitch = textureDim.x * 4;
	}
	else
	{
		ERROR_AND_DIE("We only support RGBA8 for now, format specified here is" + texture2DDesc.Format);
	}
	Texture* texture = new Texture;
	texture->m_dimensions = image.GetDimensions();
	texture->m_name = image.GetImageFilePath();

	HRESULT hResult;
	hResult = m_d3d11Device->CreateTexture2D(&texture2DDesc, &subresourceData, &texture->m_texture);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("could not create texture2D");
	}
	hResult = m_d3d11Device->CreateShaderResourceView(texture->m_texture, NULL, &texture->m_shaderResourceView);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a shader Resource View");
	}

	return texture;
}

BitmapFont* Renderer::CreateBitmapFont(char const* imageFilePathWithNoExtension)
{
	std::string imageFilePathString = imageFilePathWithNoExtension + std::string(".png");
	char const* imageFilePath = imageFilePathString.c_str();
	Texture* bitmapFontTexture = CreateTextureFromFile(imageFilePath);
	BitmapFont* newBitmapFont = new BitmapFont(imageFilePathWithNoExtension, *bitmapFontTexture);
	return newBitmapFont;
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}

BitmapFont* Renderer::CreateOrGetBitmapFont(char const* bitmapFontFilePathWithNoExtension)
{
	BitmapFont* existingBitmap = GetBitmapFontForFileName(bitmapFontFilePathWithNoExtension);
	if (existingBitmap)
	{
		return existingBitmap;
	}

	BitmapFont* newBitmapFont = CreateBitmapFont(bitmapFontFilePathWithNoExtension);
	return newBitmapFont;
}

Shader* Renderer::CreateOrGetShader(char const* shaderFilePath)
{
	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex)
	{
		if (m_loadedShaders[shaderIndex]->m_config.m_name == shaderFilePath)
		{
			return m_loadedShaders[shaderIndex];
		}
	}

	Shader* newShader = CreateShader(shaderFilePath);
	return newShader;
}

Texture* Renderer::GetTextureForFileName(char const* imageFilePath)
{
	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); ++textureIndex)
	{
		if (m_loadedTextures[textureIndex]->m_name == imageFilePath)
		{
			return m_loadedTextures[textureIndex];
		}
	}

	return nullptr;
}

BitmapFont* Renderer::GetBitmapFontForFileName(char const* bitmapFontFilePathWithNoExtension)
{
	for (int bitmapFontIndex = 0; bitmapFontIndex < m_loadedFonts.size(); ++bitmapFontIndex)
	{
		if (m_loadedFonts[bitmapFontIndex]->m_fontFilePathNameWithNoExtension == bitmapFontFilePathWithNoExtension)
		{
			return m_loadedFonts[bitmapFontIndex];
		}
	}

	return nullptr;
}

void Renderer::BindShader(Shader* shader)
{
	if (shader == nullptr)
	{
		shader = m_defaultShader;
	}

	m_d3d11DeviceContext->IASetInputLayout(shader->m_inputLayout);
	m_d3d11DeviceContext->VSSetShader(shader->m_vertexShader, NULL, NULL);
	m_d3d11DeviceContext->PSSetShader(shader->m_pixelShader, NULL, NULL);
	m_currentShader = shader;
}

Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource)
{
	HRESULT hResult;
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	
	Shader* shader = new Shader(shaderConfig);

	std::vector<unsigned char> vertexShaderByteCode;
	bool isVertexShaderCompiled = CompileShaderToByteCode(vertexShaderByteCode, shaderName, shaderSource, shaderConfig.m_vertexEntryPoint.c_str(), "vs_5_0");
	if (isVertexShaderCompiled)
	{
		hResult = m_d3d11Device->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), NULL, &shader->m_vertexShader);
		if (!SUCCEEDED(hResult))
		{
			ERROR_AND_DIE("Could not create a Vertex Shader")
		}
	}
	std::vector<unsigned char> pixelShaderByteCode;
	bool isPixelShaderCompiled = CompileShaderToByteCode(pixelShaderByteCode, shaderName, shaderSource, shaderConfig.m_pixelEntryPoint.c_str(), "ps_5_0");
	if (isPixelShaderCompiled)
	{
		hResult = m_d3d11Device->CreatePixelShader(pixelShaderByteCode.data(), pixelShaderByteCode.size(), NULL, &shader->m_pixelShader);
		if (!SUCCEEDED(hResult))
		{
			ERROR_AND_DIE("Could not create a Pixel Shader")
		}
	}

	std::string isSpriteLit = "Data/Shaders/SpriteLit";
	if (shaderName == isSpriteLit)
	{
		// Input element is a property of a vertex, to give a vertex multiple properties we make an array of vertices like in our case with a array of PCUs
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		}; // organizing the data in a way so that the GPU can understand how our Vertices are laid out in memory
		hResult = m_d3d11Device->CreateInputLayout(inputElementDesc, 4, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &shader->m_inputLayout);
	}
	else
	{
		// Input element is a property of a vertex, to give a vertex multiple properties we make an array of vertices like in our case with a array of PCUs
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		}; // organizing the data in a way so that the GPU can understand how our Vertices are laid out in memory
		hResult = m_d3d11Device->CreateInputLayout(inputElementDesc, 3, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &shader->m_inputLayout);
	}
	// Input element is a property of a vertex, to give a vertex multiple properties we make an array of vertices like in our case with a array of PCUs
	// D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	// {
	// 	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	// 	{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	// 	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	// }; // organizing the data in a way so that the GPU can understand how our Vertices are laid out in memory


	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create an input layout for how our vertices are laid out in memory")
	}

	m_loadedShaders.push_back(shader);

	return shader;
}

Shader* Renderer::CreateShader(char const* shaderName)
{
	std::string shaderFilePath = shaderName + std::string(".hlsl");
	std::string outShaderContents;
	int shaderItemsRead = FileReadToString(outShaderContents, shaderFilePath);
	if (shaderItemsRead == 0)
	{
		ERROR_AND_DIE("Failed to read the shader file: " + shaderFilePath);
	}
	Shader* shader = CreateShader(shaderName, outShaderContents.c_str());
	return shader;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	unsigned int flags1 = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDERER)
	flags1 = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pointerToAccessCompiledCode = nullptr; 
	ID3DBlob* pointerToAccessErrorMesg = nullptr;

	HRESULT hResult;
	hResult = D3DCompile(source,
		strlen(source),
		name,
		NULL,
		NULL,
		entryPoint,
		target,
		flags1,
		0,
		&pointerToAccessCompiledCode,	// shader blob (compiled code)
		&pointerToAccessErrorMesg); // error code

	if (!SUCCEEDED(hResult))
	{
		DebuggerPrintf((char const*)pointerToAccessErrorMesg->GetBufferPointer());
		ERROR_AND_DIE("Could not compile the HLSL shader source");
	}
	outByteCode.resize(pointerToAccessCompiledCode->GetBufferSize());
	memcpy(outByteCode.data(), pointerToAccessCompiledCode->GetBufferPointer(), pointerToAccessCompiledCode->GetBufferSize());

	DX_SAFE_RELEASE(pointerToAccessCompiledCode);
	DX_SAFE_RELEASE(pointerToAccessErrorMesg);

	return true;
}

VertexBuffer* Renderer::CreateVertexBuffer(size_t const size)
{
	// // Vertices are stored in the system memory, to get them on the video memory for rendering we need to create a buffer on the video memory as well
	D3D11_BUFFER_DESC vertexBufferDesc = {};									// When rendering needs our vertices Direct3D will copy contents over to the video memory.

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies data is accessible by both GPU(read only) and CPU(write only), cpu updates the resource every frame
	vertexBufferDesc.ByteWidth = (UINT)size;						// Size of the buffer to be created, It has to be same size as the array of vertices we want it to reflect
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;						// Type of buffer we want to make, in this case it's a vertex buffer, the buffer is bound to pipeline as vertex buffer
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mappable in such a way that its contents are directly effected by CPU, CPU can change it's contents

	HRESULT hResult;
	VertexBuffer* vertexBuffer = new VertexBuffer(size);
	hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &vertexBuffer->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	// hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &m_immediateVBO->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create the vertex buffer");
	}
	// m_immediateVBO->m_size = size;
	return vertexBuffer;
}

VertexBuffer* Renderer::CreateVertexBuffer(size_t const size, unsigned int stride)
{
	// // Vertices are stored in the system memory, to get them on the video memory for rendering we need to create a buffer on the video memory as well
	D3D11_BUFFER_DESC vertexBufferDesc = {};									// When rendering needs our vertices Direct3D will copy contents over to the video memory.

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies data is accessible by both GPU(read only) and CPU(write only), cpu updates the resource every frame
	vertexBufferDesc.ByteWidth = (UINT)size;						// Size of the buffer to be created, It has to be same size as the array of vertices we want it to reflect
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;						// Type of buffer we want to make, in this case it's a vertex buffer, the buffer is bound to pipeline as vertex buffer
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mappable in such a way that its contents are directly effected by CPU, CPU can change it's contents

	HRESULT hResult;
	VertexBuffer* vertexBuffer = new VertexBuffer(size, stride);
	hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &vertexBuffer->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	// hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &m_immediateVBO->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create the vertex buffer");
	}
	// m_immediateVBO->m_size = size;
	return vertexBuffer;
}

IndexBuffer* Renderer::CreateIndexBuffer(size_t const size)
{
	// // Vertices are stored in the system memory, to get them on the video memory for rendering we need to create a buffer on the video memory as well
	D3D11_BUFFER_DESC indexBufferDesc = {};									// When rendering needs our vertices Direct3D will copy contents over to the video memory.

	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies data is accessible by both GPU(read only) and CPU(write only), cpu updates the resource every frame
	indexBufferDesc.ByteWidth = (UINT)size;						// Size of the buffer to be created, It has to be same size as the array of vertices we want it to reflect
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;						// Type of buffer we want to make, in this case it's a vertex buffer, the buffer is bound to pipeline as vertex buffer
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mappable in such a way that its contents are directly effected by CPU, CPU can change it's contents

	HRESULT hResult;
	IndexBuffer* indexBuffer = new IndexBuffer(size);
	hResult = m_d3d11Device->CreateBuffer(&indexBufferDesc, NULL, &indexBuffer->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create the index buffer");
	}
	return indexBuffer;
}

void Renderer::CopyCPUToGPU(void const* data, size_t size, VertexBuffer*& vbo)
{
	if (vbo->m_size < size)
	{
		int newStride = vbo->GetStride();
		DX_SAFE_RELEASE(vbo->m_buffer);
		vbo = CreateVertexBuffer(size, newStride);
	}
	// We now have Vertices(system memory) and a vertex buffer(video memory) to place them in, now all we are required to do is copy the vertices in-to the buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;																	// Will be filled with all the imp info about the buffer including a pointer to the buffer's location
	HRESULT hResult = m_d3d11DeviceContext->Map(vbo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);	// Maps the buffer(GPU is blocked from using this buffer), giving us access to it, map_write_discard clears the prev contents of the buffers, and opens the new buffer for writting
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the buffer");
	}
	memcpy(mappedSubresource.pData, data, size);												// copies the contents from system memory to video memory
	m_d3d11DeviceContext->Unmap(vbo->m_buffer, NULL);														// GPU regains access to the buffer, CPU losses access
}

void Renderer::CopyCPUToGPU(void const* data, size_t size, IndexBuffer*& ibo)
{
	if (ibo->m_size < size)
	{
		DX_SAFE_RELEASE(ibo->m_buffer);
		ibo = CreateIndexBuffer(size);
	}
	// We now have Vertices(system memory) and a vertex buffer(video memory) to place them in, now all we are required to do is copy the vertices in-to the buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;																	// Will be filled with all the imp info about the buffer including a pointer to the buffer's location
	HRESULT hResult = m_d3d11DeviceContext->Map(ibo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);	// Maps the buffer(GPU is blocked from using this buffer), giving us access to it, map_write_discard clears the prev contents of the buffers, and opens the new buffer for writting
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the index buffer");
	}
	memcpy(mappedSubresource.pData, data, size);												// copies the contents from system memory to video memory
	m_d3d11DeviceContext->Unmap(ibo->m_buffer, NULL);														// GPU regains access to the buffer, CPU losses access
}

void Renderer::BindVertexBuffer(VertexBuffer* vbo)
{
	unsigned int vertexStride = vbo->GetStride();
	unsigned int offset = 0;
	m_d3d11DeviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &vertexStride, &offset);
	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	m_d3d11DeviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
}

ConstantBuffer* Renderer::CreateConstantBuffer(size_t const size)
{
	D3D11_BUFFER_DESC constantBufferDesc = {};

	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies GPU(read access), CPU (write access)
	constantBufferDesc.ByteWidth = (UINT)size;									// size of the buffer to be created
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;					// Type of buffer we want to make, in this case it's a constant buffer
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mapped in such a way that it's contents are directly effected by the CPU, CPU can change it's contents

	HRESULT hResult;
	ConstantBuffer* constantBuffer = new ConstantBuffer(size);
	hResult = m_d3d11Device->CreateBuffer(&constantBufferDesc, NULL, &constantBuffer->m_buffer);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a constant buffer");
	}

	return constantBuffer;
}

void Renderer::CopyCPUToGPU(void const* data, size_t size, ConstantBuffer*& cbo)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hResult = m_d3d11DeviceContext->Map(cbo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the constantBuffer");
	}
	memcpy(mappedSubresource.pData, data, size);
	m_d3d11DeviceContext->Unmap(cbo->m_buffer, NULL);
}

void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
{
	m_d3d11DeviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_d3d11DeviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	m_d3d11DeviceContext->Draw(vertexCount, vertexOffset);
}

void Renderer::DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset, int vertexOffset)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);

	m_d3d11DeviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void Renderer::DrawIndexed(int indexCount, int indexOffset, int vertexOffset)
{
	m_d3d11DeviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void Renderer::CreateAndInitializeBlendModes()
{
	D3D11_BLEND_DESC blendStateDesc = {};
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;

	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	HRESULT hResult;
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::ALPHA)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::ADDITIVE)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::OPAQUE)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}
}

void Renderer::CreateSamplerModes()
{
	HRESULT hResult;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hResult = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_samplerStates[int(SamplerMode::POINT_CLAMP)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create POINT_CLAMP sampler state");
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hResult = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_samplerStates[int(SamplerMode::BILINEAR_WRAP)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a BILINEAR_WRAP sampler state");
	}
}

void Renderer::CreateRasterizerMode()
{
	HRESULT hResult;
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;													// Fill triangles formed by vertices
	rasterizerDesc.CullMode = D3D11_CULL_BACK;													// Only draw front face
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;

	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;													
	rasterizerDesc.CullMode = D3D11_CULL_NONE;													
	
	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;	

	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;												
	rasterizerDesc.CullMode = D3D11_CULL_BACK;													

	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}
}

void Renderer::CreateDepthMode()
{
	HRESULT hResult;
	
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	hResult = m_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::ENABLED]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil state");
	}

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	hResult = m_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil state");
	}
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}

void Renderer::SetRasterizerMode(RasterizerMode rasterizerMode)
{
	m_desiredRasterizedMode = rasterizerMode;
}

void Renderer::SetDepthMode(DepthMode depthMode)
{
	m_desiredDepthMode = depthMode;
}

void Renderer::SetStatesIfChanged()
{
	if (m_blendStates[int(m_desiredBlendMode)] != m_blendState)
	{
		m_blendState = m_blendStates[int(m_desiredBlendMode)];
		float blendFactor[4] = { 0 };
		UINT sampleMask = 0xffffffff;
		m_d3d11DeviceContext->OMSetBlendState(m_blendState, blendFactor, sampleMask);
	}

	if (m_samplerStates[int(m_desiredSamplerMode)] != m_d3d11SamplerState)
	{
		m_d3d11SamplerState = m_samplerStates[int(m_desiredSamplerMode)];
		m_d3d11DeviceContext->PSSetSamplers(0, 1, &m_d3d11SamplerState);
	}

	if (m_rasterizerStates[int(m_desiredRasterizedMode)] != m_d3d11RasterizeState)
	{
		m_d3d11RasterizeState = m_rasterizerStates[int(m_desiredRasterizedMode)];
		m_d3d11DeviceContext->RSSetState(m_d3d11RasterizeState);
	}

	if (m_depthStencilStates[int(m_desiredDepthMode)] != m_depthStencilState)
	{
		m_depthStencilState = m_depthStencilStates[int(m_desiredDepthMode)];
		m_d3d11DeviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}
}

void Renderer::SetModelConstants(Mat44 const& modelMatrix, Rgba8 const& modelColor)
{
	ModelConstants modelConstants;
	modelConstants.modelMatrix = modelMatrix;
	modelColor.GetAsFloats(modelConstants.modelColor);
	CopyCPUToGPU(&modelConstants, sizeof(modelConstants), m_modelCBO);
	BindConstantBuffer(s_modelConstantsSlot, m_modelCBO);
}

void Renderer::SetLightingConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity)
{
	LightingConstants lightingConstants;

	lightingConstants.sunDirection = sunDirection;
	lightingConstants.sunIntensity = sunIntensity;
	lightingConstants.ambientIntensity = ambientIntensity;
	lightingConstants.sunDirectionPadding = Vec3(-1.f, -1.f, -1.f);
	memcpy(&lightingConstants.Lights, &m_lights, sizeof(m_lights));

	CopyCPUToGPU(&lightingConstants, sizeof(lightingConstants), m_lightingCBO);
	BindConstantBuffer(s_lightingConstantsSlot, m_lightingCBO);
}

void Renderer::SetLightAt(Light const& lightToSet, int indexToSet)
{
	bool isIndexValid = indexToSet < MAX_LIGHTS ? true : false;
	isIndexValid = isIndexValid && (indexToSet >= 0 ? true : false);
	
	if (!isIndexValid) return;

	m_lights[indexToSet] = lightToSet;
}
