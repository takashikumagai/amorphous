#ifndef __ItemStatusDebugDisplay_HPP__
#define __ItemStatusDebugDisplay_HPP__


#include <boost/weak_ptr.hpp>
#include "GameItem.hpp"
#include "../Support/DebugOutput.hpp"


class CItemStatusDebugDisplay : public CDebugItem_ResourceManager
{
	boost::weak_ptr<CGameItem> m_pItem;

public:

	CItemStatusDebugDisplay(){}

	CItemStatusDebugDisplay( boost::weak_ptr<CGameItem> pItem ) : m_pItem(pItem) {}

	~CItemStatusDebugDisplay(){}

	void SetItem( boost::weak_ptr<CGameItem> pItem ) { m_pItem = pItem; }

	void GetTextInfo()
	{
		m_TextBuffer.resize( 0 );

		boost::shared_ptr<CGameItem> pItem = m_pItem.lock();

		if( pItem )
		{
			pItem->GetStatusForDebugging( m_TextBuffer );
		}
		else
		{
			m_TextBuffer += "The item has not been set or has already been released.";
		}
	}
};



#endif /* __ItemStatusDebugDisplay_HPP__ */