#include "GameWindowManager_Win32_GL.hpp"
#include "amorphous/Graphics/OpenGL/GLGraphicsDevice.hpp"
#include "amorphous/Support/WindowMisc_Win32.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/StringAux.hpp"


namespace amorphous
{

using namespace std;


LRESULT (WINAPI *g_pMessageProcedureForGameWindow_Win32_GL)( HWND, UINT, WPARAM, LPARAM ) = NULL;

// definition of the singleton instance
GameWindowManager_Win32_GL GameWindowManager_Win32_GL::ms_SingletonInstance_;


void ReSizeGLScene(GLsizei width, GLsizei height)		// Resize and initialize the GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}


GameWindowManager_Win32_GL::GameWindowManager_Win32_GL()
{
	m_CurrentScreenMode = GameWindow::WINDOWED;

	m_hDC  = NULL;
	m_hRC  = NULL;
	m_hWnd = NULL;
	m_hInstance = NULL;
}


GameWindowManager_Win32_GL::~GameWindowManager_Win32_GL()
{
//	DIRECT3D9.Release();
//	UnregisterClass( m_ApplicationClassName.c_str(), m_WindowClassEx.hInstance );
}


/*	This code creates our OpenGL window.  Parameters are:
 *	title			- Title To Appear At The Top Of The Window
 *	width			- Width Of The GL Window Or Fullscreen Mode
 *	height			- Height Of The GL Window Or Fullscreen Mode
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)
 */

//bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
bool GameWindowManager_Win32_GL::CreateGameWindow( int iScreenWidth, int iScreenHeight, GameWindow::ScreenMode screen_mode, const std::string& app_title )
{
	LOG_FUNCTION_SCOPE();

	// Commented out: glGetString() returns NULL for all the four arguments when it is called at this point.
//	LogGLInfo();

	int bits = 16;

	const int width  = iScreenWidth;
	const int height = iScreenHeight;

	GLuint		PixelFormat;		        // Holds the results after searching for a match
	WNDCLASS	wc;					        // Windows Class Structure
	DWORD		dwExStyle;			        // Window Extended Style
	DWORD		dwStyle;			        // Window Style
	RECT		WindowRect;			        // Grabs rectangle upper left / lower right values
	WindowRect.left   =(long)0;
	WindowRect.right  =(long)width;
	WindowRect.top    =(long)0;
	WindowRect.bottom =(long)height;

	string class_name = "[[" + app_title + "]]";
	m_ClassName = class_name;

	m_hInstance			= GetModuleHandle(NULL);				// Grab an instance for our window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw on size, and own DC for window.
	wc.lpfnWndProc		= (WNDPROC) g_pMessageProcedureForGameWindow;//_Win32_GL;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;                                    // No Extra Window Data
	wc.cbWndExtra		= 0;                                    // No Extra Window Data
	wc.hInstance		= m_hInstance;                          // Set the instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);          // Load the default icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);          // Load the arrow pointer
	wc.hbrBackground	= NULL;                                 // No Background Required For GL
	wc.lpszMenuName		= NULL;                                 // We don't want a menu
	wc.lpszClassName	= class_name.c_str();                   // Set the class name

	if( !RegisterClass(&wc) )									// Attempt To Register The Window Class
	{
		LOG_PRINT_ERROR( "RegisterClass() failed. Failed to register the window class." );
		return FALSE;
	}
	
	if( screen_mode == GameWindow::FULLSCREEN )												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;                // Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;               // Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;                 // Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if( ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL )
		{
			// Use the windowed mode
			screen_mode = GameWindow::WINDOWED;		// Windowed Mode Selected.
		}
	}

	if( screen_mode == GameWindow::FULLSCREEN )						// Are we still in fullscreen mode?
	{
		dwExStyle = WS_EX_APPWINDOW;                                // Window Extended Style
		dwStyle   = WS_POPUP;		                                // Windows Style
		ShowCursor(FALSE);			                                // Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;             // Window Extended Style
		dwStyle   = WS_OVERLAPPEDWINDOW;                            // Windows Style
	}												                

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(m_hWnd=CreateWindowEx( dwExStyle,							// Extended Style For The Window
								class_name.c_str(),	                // Class Name
								app_title.c_str(),		            // Window Title
								dwStyle |				            // Defined Window Style
								WS_CLIPSIBLINGS |		            // Required Window Style
								WS_CLIPCHILDREN,		            // Required Window Style
								0, 0,					            // Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								m_hInstance,						// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow(); // Reset The Display
		LOG_PRINT_ERROR( "CreateWindowEx() failed." );
		return FALSE;
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if( !(m_hDC=GetDC(m_hWnd)) )                    // Did we get a Device Context?
	{
		KillGLWindow();								// Reset the display
		LOG_PRINT_ERROR( "Can't create a GL Device Context." );
		return FALSE;
	}

	if( !(PixelFormat=ChoosePixelFormat(m_hDC,&pfd)) )	// Did Windows find a matching pixel format?
	{
		KillGLWindow(); // Reset the display
		LOG_PRINT_ERROR( "Can't find a suitable PixelFormat." );
		return FALSE;
	}

	if( !SetPixelFormat(m_hDC,PixelFormat,&pfd) )     // Are we able to set the pixel format?
	{
		KillGLWindow(); // Reset The Display
		LOG_PRINT_ERROR( "Can't set the PixelFormat." );
		return FALSE;
	}

	if (!(m_hRC=wglCreateContext(m_hDC)))           // Are we able to get a rendering context?
	{
		KillGLWindow();								// Reset The Display
		LOG_PRINT_ERROR( "Can't create a GL Rendering Context." );
		return FALSE;
	}

	if(!wglMakeCurrent(m_hDC,m_hRC))					// Try to activate the rendering Context
	{
		KillGLWindow();								// Reset the display
		LOG_PRINT_ERROR( "Can't activate the GL Rendering Context." );
		return FALSE;
	}

	// Called from here, glGetString() returns valid string values...
	// but decided to call LogGLInfo() at the beginning of CGLGraphicsDevice::Init()
	// because most of the gl*() functions are called inside CGLGraphicsDevice's
	// member functions.
//	LogGLInfo();

	ShowWindow(m_hWnd,SW_SHOW);						// Show the window
	SetForegroundWindow(m_hWnd);						// Slightly higher priority
	SetFocus(m_hWnd);									// Sets keyboard focus to the window
	ReSizeGLScene(width, height);					// Set up our perspective GL screen

	if( !GLGraphicsDevice().Init( iScreenWidth, iScreenHeight, (screen_mode == ScreenMode::WINDOWED) ? ScreenMode::WINDOWED : ScreenMode::FULLSCREEN ) )
	{
		KillGLWindow();								// Reset the display
		LOG_PRINT_ERROR( "GLGraphicsDevice::Init() failed." );
		return FALSE;								// Return FALSE
	}

	// initialize OpenGL extensions
//	initExtensions( m_hDC );

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		LOG_PRINTF_ERROR(( "glewInit error: %s", glewGetErrorString(err)));
		return FALSE;
	}

	return Init( iScreenWidth, iScreenHeight, screen_mode );
}


void GameWindowManager_Win32_GL::ChangeScreenSize( int iNewScreenWidth,
												 int iNewScreenHeight,
												 bool bFullScreen )
{/*
	// check if the requested window size has a valid aspect ratio
	// (width : height) must be 4:3
//	if( 0.01f < fabs( (float)iNewScreenHeight / (float)iNewScreenWidth - 0.75f ) )
//		return;


	// release all the graphic resources ( textures, vertex buffers, mesh models, and so on )
	GraphicsComponentCollector::Get()->ReleaseGraphicsResources();

	if( !DIRECT3D9.ResetD3DDevice( m_hWnd, iNewScreenWidth, iNewScreenHeight, bFullScreen ) )
	{
		// the requested resolution is not available - restore the current settings
		bool bCurrentModeFullScreen = (m_CurrentScreenMode == GameWindow::FULLSCREEN) ? true : false;
		DIRECT3D9.ResetD3DDevice( m_hWnd, m_iCurrentScreenWidth, m_iCurrentScreenHeight, bCurrentModeFullScreen );
	}

	// update the current resolution and the screen mode
	m_iCurrentScreenWidth  = iNewScreenWidth;
	m_iCurrentScreenHeight = iNewScreenHeight;
	m_CurrentScreenMode   = GameWindow::WINDOWED; // TODO: use the new screen mode


	GraphicsParameters param;
	param.ScreenWidth  = iNewScreenWidth;
	param.ScreenHeight = iNewScreenHeight;
	param.bWindowed = (!bFullScreen);

	// notify all the graphics components to load their resources
	GraphicsComponentCollector::Get()->LoadGraphicsResources( param );

	// notify changes to all the game components
//	GAMECOMPONENTCOLLECTOR.AdaptToNewScreenSize();

	if( m_CurrentScreenMode == GameWindow::WINDOWED )
	{
		ChangeClientAreaSize( m_hWnd, m_iCurrentScreenWidth, m_iCurrentScreenHeight );
	}

	// adjust the position of the window so that the game screen appear in the middle of the display
	int iDesktopWidth, iDesktopHeight;
	GetCurrentResolution( &iDesktopWidth, &iDesktopHeight );
*/
}


void GameWindowManager_Win32_GL::OnMainLoopFinished()
{
	BOOL ret = SwapBuffers( m_hDC );
	if( !ret )
	{
		DWORD error = GetLastError();
		LOG_PRINTF_ERROR(( "SwapBuffers() failed (error: %d).", (int)error ));
	}
}


/*
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing The Base Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */


bool	active=TRUE;		// Window Active Flag Set To TRUE By Default


/// Properly Kill The Window
Result::Name GameWindowManager_Win32_GL::KillGLWindow()
{
	if( m_CurrentScreenMode == GameWindow::FULLSCREEN )	// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (m_hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(m_hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		m_hRC = NULL;										// Set RC To NULL
	}

	if (m_hDC && !ReleaseDC(m_hWnd,m_hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hDC=NULL;										// Set DC To NULL
	}

	if (m_hWnd && !DestroyWindow(m_hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release m_hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hWnd=NULL;										// Set m_hWnd To NULL
	}

	if (!UnregisterClass(m_ClassName.c_str(),m_hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hInstance=NULL;									// Set hInstance To NULL
	}

	return Result::SUCCESS;
}


} // namespace amorphous
