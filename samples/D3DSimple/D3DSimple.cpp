#ifdef MEMORY_LEAKS_CHECK
	#pragma message("build will Check for Memory Leaks!")
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
    inline void* operator new(size_t size, const char *file, int line)
    {
       return ::operator new(size, 1, file, line);
    }

    inline void __cdecl operator delete(void *ptr, const char *file, int line) 
    {
       ::operator delete(ptr, _NORMAL_BLOCK, file, line);
    }

    #define DEBUG_NEW new( __FILE__, __LINE__)
    #define MALLOC_DBG(x) _malloc_dbg(x, 1, __FILE__, __LINE__);
    #define malloc(x) MALLOC_DBG(x)
    #define new DEBUG_NEW
#endif

#include <stdio.h>

#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>

#include "FxParser.h"

HDC         g_hDC       = NULL;
HGLRC       g_hRC       = NULL;
HWND        g_hWnd      = NULL;
static int  g_curX      = 0;
static int  g_curY      = 0;

bool        g_bCtrl     = false;
bool        g_bShift    = false;
int         g_renderCnt = 0;

int g_width = 800;
int g_height = 600;

// D3D Objects
ID3D11Device *				g_d3dDevice				= NULL;
ID3D11Device1 *				g_d3dDevice1			= NULL;
ID3D11DeviceContext *		g_immediateContext		= NULL;
ID3D11DeviceContext1 *		g_immediateContext1		= NULL;
IDXGISwapChain *			g_swapChain				= NULL;
IDXGISwapChain1 *			g_swapChain1			= NULL;
ID3D11RenderTargetView *	g_renderTargetView		= NULL;
ID3D11Texture2D *			g_texture				= NULL;
ID3D11Texture2D *			g_stagingTexture		= NULL;
ID3D11ShaderResourceView *	g_textureRV				= NULL;
ID3D11SamplerState *		g_samplerLinear			= NULL;
ID3D11VertexShader *		g_vertexShader			= NULL;
ID3D11PixelShader *			g_pixelShader			= NULL;

// FX Objects
nvFX::IContainer *			fx_EffectScene			= NULL;
nvFX::ITechnique *			fx_TechScene			= NULL;


void render()
{
#if 1

	nvFX::PassInfo pr;
	memset(&pr, 0, sizeof(pr));

	if(fx_TechScene)
	{
		int np = fx_TechScene->getNumPasses();
		for(int i=0; i<np; i++)
		{
			nvFX::IPass* scenePass = fx_TechScene->getPass(i, &pr);
			if(!pr.bActive)
				continue;
			if(!scenePass->isValidated())
			{
				continue;
			}

			pr.renderingGroup = 0; // set back to 0 before each pass. So no persistent value across passes
			scenePass->execute(&pr);
		}
	}

#else

	const FLOAT blue[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_immediateContext->ClearRenderTargetView(g_renderTargetView, blue);
//	g_immediateContext->Draw(3, 0);

#endif

	g_swapChain->Present(0, 0);
}

void printMessage(int level, const char * txt)
{
	OutputDebugString(txt);
}
void nvFXErrorMsg(const char * txt)
{
	OutputDebugString(txt);
}
void nvFXMsg(const char * txt)
{
	OutputDebugString(txt);
}

bool initFx()
{
	nvFX::setErrorCallback(nvFXErrorMsg);
	nvFX::setMessageCallback(nvFXMsg);

	nvFX::setDevice(g_d3dDevice);
	nvFX::setDefaultBackBuffer(g_renderTargetView);
	fx_EffectScene = nvFX::IContainer::create();

#if 0
	bool bRes = nvFX::loadEffectFromFile(fx_EffectScene, "Simple.d3dfx");
	if(!bRes)
		bRes = nvFX::loadEffectFromFile(fx_EffectScene, SOURCE_DIR "/Simple.d3dfx");
#else
	bool bRes = nvFX::loadEffectFromFile(fx_EffectScene, SOURCE_DIR "/Simple.d3dfx");
#endif
	if(!bRes)
	{
		OutputDebugStringA("Effect file not found. Make sure it is close to the exe file...");
		return false;
	}
	fx_TechScene = fx_EffectScene->findTechnique(0);
	fx_TechScene->validate();

	return true;
}

bool initBase()
{
	return true;
}

bool initD3D()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_d3dDevice, &featureLevel, &g_immediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_d3dDevice, &featureLevel, &g_immediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return false;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));

		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);

			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}

			dxgiDevice->Release();
		}
	}

	if (FAILED(hr))
		return false;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));

	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_d3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_d3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_immediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_immediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = g_width;
		sd.Height = g_height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_d3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_swapChain1);

		if (SUCCEEDED(hr))
		{
			hr = g_swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_swapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = g_width;
		sd.BufferDesc.Height = g_height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_d3dDevice, &sd, &g_swapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return false;

	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = g_width;
	textureDesc.Height = g_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	hr = g_d3dDevice->CreateTexture2D(&textureDesc, NULL, &g_texture);

	if (FAILED(hr))
	{
		return false;
	}

	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.BindFlags = 0;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = g_d3dDevice->CreateTexture2D(&textureDesc, NULL, &g_stagingTexture);

	if (FAILED(hr))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = g_d3dDevice->CreateShaderResourceView(g_texture, &shaderResourceViewDesc, &g_textureRV);

	if (FAILED(hr))
	{
		return false;
	}

	// Create a render target view
	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

	if (FAILED(hr))
		return false;

	hr = g_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
	backBuffer->Release();

	if (FAILED(hr))
		return false;

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)g_width;
	vp.Height = (FLOAT)g_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_immediateContext->RSSetViewports(1, &vp);

	// Create the vertex shader
//	hr = g_d3dDevice->CreateVertexShader(drawFullScreenTriangleVS, sizeof(drawFullScreenTriangleVS), nullptr, &g_vertexShader);

	if (FAILED(hr))
	{
		return false;
	}

	// Set the input layout
	g_immediateContext->IASetInputLayout(NULL);

	// Create the pixel shader
//	hr = g_d3dDevice->CreatePixelShader(ps_copy, sizeof(ps_copy), nullptr, &g_pixelShader);

	if (FAILED(hr))
		return false;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_d3dDevice->CreateSamplerState(&sampDesc, &g_samplerLinear);

	if (FAILED(hr))
		return false;

	// Set primitive topology
	g_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render a triangle
	g_immediateContext->VSSetShader(g_vertexShader, nullptr, 0);
	g_immediateContext->PSSetShader(g_pixelShader, nullptr, 0);

	g_immediateContext->PSSetShaderResources(0, 1, &g_textureRV);
	g_immediateContext->PSSetSamplers(0, 1, &g_samplerLinear);
	g_immediateContext->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

	return true;
}

#define D3D_SAFE_RELEASE(obj) \
	if (obj) \
	{ \
		obj->Release(); \
		obj = nullptr; \
	}

void destroyD3D()
{
	if (g_immediateContext)
		g_immediateContext->ClearState();

	D3D_SAFE_RELEASE(g_samplerLinear);
	D3D_SAFE_RELEASE(g_vertexShader);
	D3D_SAFE_RELEASE(g_pixelShader);
	D3D_SAFE_RELEASE(g_textureRV);
	D3D_SAFE_RELEASE(g_stagingTexture);
	D3D_SAFE_RELEASE(g_texture);
	D3D_SAFE_RELEASE(g_renderTargetView);
	D3D_SAFE_RELEASE(g_swapChain1);
	D3D_SAFE_RELEASE(g_swapChain);
	D3D_SAFE_RELEASE(g_immediateContext1);
	D3D_SAFE_RELEASE(g_immediateContext);
	D3D_SAFE_RELEASE(g_d3dDevice1);
	D3D_SAFE_RELEASE(g_d3dDevice);
}

#undef D3D_SAFE_RELEASE

void destroyBase()
{
}

LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
	UINT   msg, 
	WPARAM wParam, 
	LPARAM lParam )
{
	bool bRes = false;
	//
	// Pass the messages to our UI, first
	//
	if(!bRes) switch( msg )
	{
	case WM_PAINT:
		g_renderCnt++;
		break;
	case WM_KEYUP:
		g_renderCnt++;
		switch( wParam )
		{
		case VK_CONTROL:
			g_bCtrl = false;
			break;
		case VK_SHIFT:
			g_bShift = false;
			break;
		}
		break;
	case WM_KEYDOWN:
		g_renderCnt++;
		switch( wParam )
		{
		case VK_CONTROL:
			g_bCtrl = true;
			break;
		case VK_SHIFT:
			g_bShift = true;
			break;
		default:
			//keyboard_(wParam, g_curX, g_curY);
			break;
		}
		break;
	case WM_SIZE:
		//reshape(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc( g_hWnd, msg, wParam, lParam );
}

int WINAPI WinMain(    HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow )
{
#ifdef MEMORY_LEAKS_CHECK
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 
	_CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW);
#endif
	//initNSight();

	WNDCLASSEX winClass;
	MSG        uMsg;

	memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName	= "MY_WINDOWS_CLASS";
	winClass.cbSize			= sizeof(WNDCLASSEX);
	winClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc	= WindowProc;
	winClass.hInstance		= hInstance;
	winClass.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
	winClass.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
	winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName	= NULL;
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;

	if (!RegisterClassEx(&winClass))
		return E_FAIL;

	const int windowSize[2] = {800, 600};
	const int windowPos[2] = { (GetSystemMetrics(SM_CXSCREEN) - windowSize[0])/2, (GetSystemMetrics(SM_CYSCREEN) - windowSize[1])/2 };

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS",
		"D3D Sample",
		WS_OVERLAPPEDWINDOW, windowPos[0], windowPos[1], windowSize[0], windowSize[1], NULL, NULL, 
		hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

	ShowWindow( g_hWnd, nCmdShow );
	UpdateWindow( g_hWnd );

	// Initialize the very base of OpenGL
	if(initBase())
	{
		// Initialize more general stuff... typically what can be declared when using GLUT
		initD3D();
		initFx();

		//---------------------------------------------------------------------------
		// Message pump
		while( uMsg.message != WM_QUIT )
		{
			if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
			{ 
				TranslateMessage( &uMsg );
				DispatchMessage( &uMsg );
			}
			else 
			{
				//idle();
#if 0
				render();
#else
				if(g_renderCnt > 0)
				{
					g_renderCnt--;
					render();
				} else
					Sleep(10);
#endif
			}
		}
	}
	destroyD3D();
	if( g_hRC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
	destroyBase();
	UnregisterClass( "MY_WINDOWS_CLASS", hInstance );

#ifdef MEMORY_LEAKS_CHECK
	_CrtDumpMemoryLeaks();
#endif
	return (int)uMsg.wParam;
}