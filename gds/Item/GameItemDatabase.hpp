#ifndef  __GAMEITEMDATABASE_H__
#define  __GAMEITEMDATABASE_H__


#include "Support/memory_helpers.hpp"

#include "Support/Serialization/Serialization.hpp"
using namespace GameLib1::Serialization;

class CGameItem;


class CGameItemInfo : public IArchiveObjectBase
{
public:

	std::string strItemName;
	std::string strFilename;

	CGameItem *pItem;

	/// indicates if the item has been loaded on the memory
//	bool bIsLoaded;

	CGameItemInfo() { pItem = NULL; /*bIsLoaded = false;*/ }

//	~CGameItemInfo() { ReleaseItem(); }

	CGameItem *GetItem( const int quantity );

	/// release the item if it is currently loaded
	/// must be called explicitly by the owner before destroying the object
	void ReleaseItem() { SafeDelete( pItem ); }

	void Serialize( IArchive& ar, const unsigned int version );
};


class CGameItemDatabase : public IArchiveObjectBase
{
	std::vector<CGameItemInfo> m_vecGameItemInfo;

public:

	CGameItemDatabase() {}

	~CGameItemDatabase();

	void AddItemInfo( const std::string& strFilename, const std::string& strItemName );

	bool LoadFromFile( const char *pcFilename );

    bool SaveToFile( const char *pcFilename );

	void Serialize( IArchive& ar, const unsigned int version );

	/// returns item(s) with the specified name
	/// returns NULL if the specified item is not found
	CGameItem *GetItem( const char *pItemName, int quantity );

	/// save an item object as a binary file.
	/// not used during runtime
	void SaveItem( const char *pFilename, CGameItem *pItem );

	void SortItemsInAlphabeticalOrder();
};



#endif  /*  __GAMEITEMDATABASE_H__  */
