#include "GameWindowManager_Win32.hpp"
#include "GameWindowManager_Win32_GL.hpp"
#include "amorphous/Graphics/GraphicsDevice.hpp"
#include "amorphous/Graphics/OpenGL/GLInitialization.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/WindowMisc_Win32.hpp"

#ifdef BUILD_WITH_DIRECT3D
#include "GameWindowManager_Win32_D3D.hpp"
#include "amorphous/Graphics/Direct3D/D3DInitialization.hpp"
#endif /* BUILD_WITH_DIRECT3D */


namespace amorphous
{


static GameWindowManager_Win32 *g_pGameWindowManager_Win32 = nullptr;


GameWindowManager_Win32::GameWindowManager_Win32()
:
m_hWnd(NULL)
{
	memset( &m_WindowClassEx, 0, sizeof(WNDCLASSEX) );
}


void GameWindowManager_Win32::Release()
{
	if( 0 < m_ApplicationClassName.length() )
	{
	   UnregisterClass( m_ApplicationClassName.c_str(), m_WindowClassEx.hInstance );
	   m_ApplicationClassName = "";
	}
}


void GameWindowManager_Win32::SetWindowTitleText( const std::string& text )
{
	BOOL res = SetWindowText( m_hWnd, text.c_str() );
	if( !res )
	{
//		LOG_PRINT_ERROR( "Failed to set the window title text." );
	}
}


bool GameWindowManager_Win32::IsMouseCursorInClientArea()
{
	long frame_w = 0, frame_h = 0;
	GetNonClientAreaSize( m_hWnd, frame_w, frame_h );

	RECT window_rect;
	GetWindowRect( m_hWnd, &window_rect );

	// TODO: accurate client rect position
	RECT offset_window_rect = window_rect;
	offset_window_rect.top   += frame_h;
	offset_window_rect.left  += frame_w;
	offset_window_rect.right -= frame_w;

	POINT pt;
	GetCursorPos( &pt );

	if( offset_window_rect.top <= pt.y
	 && pt.y <= offset_window_rect.bottom
	 && offset_window_rect.left <= pt.x
	 && pt.x <= offset_window_rect.right )
	{
		return true;
	}
	else
		return false;
}


void GameWindowManager_Win32::SetWindowLeftTopCornerPosition( int left, int top )
{
	// set the position
	// - use SWP_NOSIZE flag to ignore window size parameters
	::SetWindowPos( m_hWnd, HWND_TOP, left, top, 0, 0, SWP_NOSIZE );
}


void GameWindowManager_Win32::MainLoop( ApplicationCore& app )
{
    // Enter the message loop
    MSG msg;
    ZeroMemory( &msg, sizeof(msg) );
    while( msg.message!=WM_QUIT && app.IsAppExitRequested() == false )
    {
        if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
        {
			CScopeProfile sp( "Windows message translation and dispatch" );

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
		{
			app.UpdateFrame();

			OnMainLoopFinished();
		}
	}	
}


Result::Name SelectGraphicsLibrary_Win32( const std::string& graphics_library_name, GameWindowManager*& pGameWindowManager )
{
	if( graphics_library_name == "OpenGL" )
	{
		g_pGameWindowManager_Win32 = &GetGameWindowManager_Win32_GL();
		InitializeOpenGLClasses();
	}
#ifdef BUILD_WITH_DIRECT3D
	else if( graphics_library_name == "Direct3D" )
	{
		g_pGameWindowManager_Win32 = &GetGameWindowManager_Win32_D3D();
		InitializeDirect3DClasses();
	}
#endif /* BUILD_WITH_DIRECT3D */
	else
	{
		LOG_PRINT_ERROR( "Unsupported graphics library: " + graphics_library_name );
		g_pGameWindowManager_Win32 = nullptr;

		return Result::INVALID_ARGS;
	}

//	PrimitiveRenderer().Init();

	pGameWindowManager = g_pGameWindowManager_Win32;

	return Result::SUCCESS;
}


GameWindowManager_Win32& GetGameWindowManager_Win32()
{
//	return GameWindowManager_Win32::ms_SingletonInstance_;
	return *g_pGameWindowManager_Win32;
}


} // namespace amorphous
