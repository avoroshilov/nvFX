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

#include <windows.h>

HDC         g_hDC       = NULL;
HGLRC       g_hRC       = NULL;
HWND        g_hWnd      = NULL;
static int  g_curX      = 0;
static int  g_curY      = 0;

bool        g_bCtrl     = false;
bool        g_bShift    = false;
int         g_renderCnt = 0;

bool initBase()
{
	return true;
}

void initD3D()
{
}

void destroyD3D()
{
}

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

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS",
		"Viewer",
		WS_OVERLAPPEDWINDOW, 0, 0, windowSize[0], windowSize[1], NULL, NULL, 
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
				if(g_renderCnt > 0)
				{
					g_renderCnt--;
					//display();
				} else
					Sleep(10);
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