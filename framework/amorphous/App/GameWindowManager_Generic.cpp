#include "GameWindowManager_Generic.hpp"
//#include "amorphous/Graphics/GraphicsDevice.hpp"
#include "amorphous/Graphics/OpenGL/GLInitialization.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
//#include "amorphous/Support/WindowMisc_Win32.hpp"


namespace amorphous
{


GameWindowManager_Generic GameWindowManager_Generic::ms_SingletonInstance_;


Result::Name SelectGraphicsLibrary_Generic( const std::string& graphics_library_name, GameWindowManager*& pGameWindowManager )
{
	if( graphics_library_name == "OpenGL" )
	{
		InitializeOpenGLClasses();
	}
	else
	{
		LOG_PRINT_ERROR( "Unsupported graphics library: " + graphics_library_name );

		return Result::INVALID_ARGS;
	}

	pGameWindowManager = &GetGameWindowManager_Generic();

	return Result::SUCCESS;
}

bool GameWindowManager_Generic::CreateGameWindow(
    int iScreenWidth,
    int iScreenHeight,
    GameWindow::ScreenMode screen_mode,
    const std::string& app_title
    )
{
#ifdef BUILD_WITH_X11_LIBS
   
    m_pX11GLWindow.reset( new X11_GLWindow );

    m_pX11GLWindow->Init( iScreenWidth, iScreenHeight, app_title.c_str() );

#endif // BUILD_WITH_X11_LIBS

    return true;
}

void GameWindowManager_Generic::MainLoop( ApplicationCore& app )
{
#ifdef BUILD_WITH_X11_LIBS

	if(m_pX11GLWindow)
	{
		m_pX11GLWindow->MainLoop(app);
	}

#endif // BUILD_WITH_X11_LIBS
}

GameWindowManager_Generic& GetGameWindowManager_Generic()
{
	return GameWindowManager_Generic::GetSingletonInstance();
}

} // namespace amorphous
