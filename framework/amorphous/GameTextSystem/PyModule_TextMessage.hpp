#ifndef __PYMODULE_TEXTMESSAGE_H__
#define __PYMODULE_TEXTMESSAGE_H__


#include "fwd.hpp"
#include "amorphous/Script/fwd.hpp"
#include "amorphous/Script/PythonHeaders.hpp"


namespace amorphous
{


//extern void SetTextMessageManager( CTextMessageManager* pTextMessageMgr );
extern void SetTextMsgMgrForScriptCallback( CTextMessageManager* pTextMessageMgr );

void RegisterPythonModule_TextMessage( PythonScriptManager& mgr );

} // namespace amorphous



#endif  /*  __PYMODULE_TEXTMESSAGE_H__  */
