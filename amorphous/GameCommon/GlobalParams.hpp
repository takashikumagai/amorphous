#ifndef __GLOBALPARAMS_H__
#define __GLOBALPARAMS_H__


#include <stdio.h>
#include "Support/ParamLoader.hpp"
#include "Support/Log/LogOutputBase.hpp"


namespace amorphous
{


//>>>========================== default config file ==========================
/*
resolution 800	600
fullscreen	yes
*/
//<<<========================== default config file ==========================


/**
 - Global settings for a game application
   - Always loaded from a text file.
   - The application can let the user modify the content of the file
     to configure the settings.

*/
class GlobalParams
{
public:

	bool FullScreen;
	int ScreenWidth;
	int ScreenHeight;

	/// top-left corner of the window
	/// - valid only in windowed mode
	/// - set to -1 if not specified,
	/// and the window will be placed in the center of the desktop
	int WindowLeftPos;
	int WindowTopPos;

	std::string ScreenshotImageFormat;
	int ScreenshotResolutionWidth;
	int ScreenshotResolutionHeight;
//	std::string ScreenshotOutputDirectory;

	int LogVerbosity;

	std::string AudioLibraryName;

	std::string GraphicsLibraryName;

private:

	void LoadLogVerbosity( ParamLoader& loader );

public:

	GlobalParams()
		:
	FullScreen(false),
	ScreenWidth(1280),
	ScreenHeight(720),
	WindowLeftPos(-1),
	WindowTopPos(-1),
	ScreenshotImageFormat( "bmp" ),
	ScreenshotResolutionWidth( -1 ),
	ScreenshotResolutionHeight( -1 ),
	LogVerbosity(WL_WARNING),
	AudioLibraryName("OpenAL"),
	GraphicsLibraryName("Direct3D")
	{}

	bool LoadFromFile( const std::string& filename )
	{
		ParamLoader loader( filename );

		if( !loader.IsReady() )
			return false;

		loader.LoadBoolParam( "fullscreen", "yes/no", FullScreen );
		loader.LoadParam( "resolution",               ScreenWidth, ScreenHeight );
		loader.LoadParam( "window_pos",               WindowLeftPos, WindowTopPos );
		loader.LoadParam( "screenshot_format",        ScreenshotImageFormat );
		loader.LoadParam( "screenshot_resolution",    ScreenshotResolutionWidth, ScreenshotResolutionHeight );
		loader.LoadParam( "audio_library",            AudioLibraryName );

		LoadLogVerbosity( loader );

		return true;
	}
};


extern GlobalParams g_GlobalParams;

} // namespace amorphous



#endif  /*  __GLOBALPARAMS_H__  */