
#ifndef  __GAMETASK_GLOBALSTAGELOADER_H__
#define  __GAMETASK_GLOBALSTAGELOADER_H__


#include "GameTask.hpp"


namespace amorphous
{

class CInputHandler;
class CGM_DialogManager;
class CGM_StdControlRendererManager;
class TitleEventHandler;
class FontBase;


class CGameTask_GlobalStageLoader : public CGameTask
{
	// handles the input to the player ship
//	CInputHandler *m_pInputHandler;
//	CGM_DialogManagerSharedPtr m_pDialogManager;
//	CGM_ControlRendererManagerSharedPtr m_pUIRenderManager;
//	TitleEventHandler* m_pEventHandler;

	FontBase *m_pFont;

	bool m_bRendered;

	bool m_bStageLoaded;

	static std::string ms_strStageTask;

public:

	CGameTask_GlobalStageLoader();
	virtual ~CGameTask_GlobalStageLoader();

	virtual int FrameMove( float dt );
	virtual void Render();

	void ReleaseGraphicsResources();
	void LoadGraphicsResources( const GraphicsParameters& rParam );

	static void SetTaskForGlobalStage( const std::string& next_task_name ) { ms_strStageTask = next_task_name; }
};

} // namespace amorphous



#endif  /*  __GAMETASK_GLOBALSTAGELOADER_H__  */
