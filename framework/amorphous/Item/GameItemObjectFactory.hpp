#ifndef __GameItemObjectFactory_H__
#define __GameItemObjectFactory_H__


#include "amorphous/Support/Serialization/Serialization.hpp"
#include "amorphous/Support/Serialization/ArchiveObjectFactory.hpp"


namespace amorphous
{
using namespace serialization;


class GameItem;


//===========================================================================
// GameItemObjectFactory
//===========================================================================

class GameItemObjectFactory : public IArchiveObjectFactory
{
public:

	GameItem *CreateGameItem( const unsigned int id );

	IArchiveObjectBase *CreateObject( const unsigned int id );
};

} // namespace amorphous



#endif  /* __GameItemObjectFactory_H__   */
