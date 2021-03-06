#include "GameTextSystem.hpp"

#include <stdlib.h>
#include "amorphous/Graphics/Font/FontBase.hpp"
#include "amorphous/Support/memory_helpers.hpp"


namespace amorphous
{

using namespace std;


//===========================================================================
// CGameTextComponent
//===========================================================================


/*
//===========================================================================
// CGameTextManager
//===========================================================================

CGameTextManager::CGameTextManager()
{
	m_iNumGameTextSets = 0;
	m_paGameTextSet = NULL;
}

CGameTextManager::~CGameTextManager()
{
	Release();
}

void CGameTextManager::Release()
{
	SafeDeleteArray( m_paGameTextSet );
	m_iNumGameTextSets = 0;
}

bool CGameTextManager::LoadTextDataFromFile( char* pcFilename )
{
	FILE *fp = fopen( pcFilename, "rb" );

	if(!fp)
		return false;	// requested text data file was not found

	fread( &m_iNumGameTextSets, sizeof(int), 1, fp );

	m_paGameTextSet = new CGameTextSet [m_iNumGameTextSets];

	int i;
	for( i=0; i<m_iNumGameTextSets; i++ )
	{
		m_paGameTextSet[i].LoadFromFile(fp);
	}

	fclose(fp);
	return true;
}

int CGameTextManager::GetIndexForGameTextSet( char* pcTag )
{
	int i;
	for( i=0; i<m_iNumGameTextSets; i++ )
	{
		if( strcmp(m_paGameTextSet[i].GetTag(), pcTag) == 0 )
			return i;
	}
	return TEXTSET_INVALID_INDEX;
}
*/

} // namespace amorphous
