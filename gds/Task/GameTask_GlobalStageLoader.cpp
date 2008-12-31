
#include "GameTask_GlobalStageLoader.h"
#include "GameTask_Stage.h"
#include "GameTask_StageSelect.h"
#include "Stage/Stage.h"

#include "3DCommon/Direct3D9.h"
#include "3DCommon/2DRect.h"
#include "3DCommon/Font/Font.h"

#include "Support/Timer.h"

#include "GameInput/InputHub.h"

//#include "Sound/SoundManager.h"

#include "Support/memory_helpers.h"
#include "Support/Log/StateLog.h"
#include "Support/Log/DefaultLog.h"

#include "UI.h"
#include "UI/InputHandler_Dialog.h"


std::string CGameTask_GlobalStageLoader::ms_strStageTask = "Stage";


CGameTask_GlobalStageLoader::CGameTask_GlobalStageLoader()
{
	m_bRendered = false;

	m_bStageLoaded = false;

	int w = 16 * GetScreenWidth() / 800;
	int h = w * 2;
	m_pFont = new CFont( "Arial", w, h );
}


CGameTask_GlobalStageLoader::~CGameTask_GlobalStageLoader()
{
	SafeDelete( m_pFont );

//	INPUTHUB.PopInputHandler();
//	SafeDelete( m_pInputHandler );
//	SafeDelete( m_pEventHandler );
}


int CGameTask_GlobalStageLoader::FrameMove( float dt )
{
	int ret = CGameTask::FrameMove(dt);
	if( ret != ID_INVALID )
		return ret;

	if( !m_bRendered )
		return CGameTask::ID_INVALID;
	
	if( m_bStageLoaded )
		return CGameTask::ID_INVALID;

	// load the global stage
	if( 0 < GetGlobalStageScriptFilename().length() )
	{
		InitAnimatedGraphicsManager();

		// set animated graphics manager here
		// since init routines of the scripts use graphics manager
		SetAnimatedGraphicsManagerForScript();

		LoadStage( GetGlobalStageScriptFilename() );
	}
	else
	{
		PrintLog( "CGameTask_GlobalStageLoader::CGameTask_Stage() - no global stage has been specified" );
		return CGameTask::ID_PREVTASK;
	}

	m_bStageLoaded = true;


	//
	// stage has been loaded
	//

	// move on to the stage task
	RequestTaskTransition( ms_strStageTask, 0.0f, 0.0f, 0.0f );

	return CGameTask::ID_INVALID;
}


void CGameTask_GlobalStageLoader::Render()
{
	// render stage select dialog
//	m_pDialogManager->Render( dt );

	if( m_pFont )
	{
		C2DRect rect( 0,0,0,0, 0xFF000000 );
		rect.Draw();

		const string& script_name = GetGlobalStageScriptFilename();
		int font_width = m_pFont->GetFontWidth();
		int font_height = m_pFont->GetFontHeight();
		int x = GetScreenWidth() / 2 - (int)script_name.length() * font_width / 2;
		int y = GetScreenHeight() / 2 - font_height / 2;
		m_pFont->SetFontColor( 0xFFFFFFFF );
		m_pFont->DrawText( script_name, x, y );
	}

	m_bRendered = true;
}


void CGameTask_GlobalStageLoader::ReleaseGraphicsResources()
{
}


void CGameTask_GlobalStageLoader::LoadGraphicsResources( const CGraphicsParameters& rParam )
{
}

