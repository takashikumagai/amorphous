#ifndef  __EntityGroupHandle_H__
#define  __EntityGroupHandle_H__


#include "fwd.hpp"
#include "amorphous/Support/Serialization/Serialization.hpp"


namespace amorphous
{
using namespace serialization;


#define ENTITY_GROUP_ID_UNINITIALIZED -2
#define ENTITY_GROUP_INVALID_ID       -1
#define ENTITY_GROUP_MIN_ID            0
#define ENTITY_GROUP_MAX_ID           15


/**
 about entity group
 - provided to control collisions between entities
 - valid range: [ ENTITY_GROUP_MIN_ID, ENTITY_GROUP_MAX_ID ]
   - user is expected to start id with ENTITY_GROUP_MIN_ID and increment subsequent ids
   - e.g.)
     enum eEntityGroup
	 {
		GRP_GENERAL = ENTITY_GROUP_MIN_ID,
		GRP_PLAYER,
		GRP_PLAYER_PROJECTILE,
		GRP_CPU,
		GRP_CPU_PROJECTILE,
		NUM_ENTITY_GROUPS
	 };

   - ENTITY_GROUP_MIN_ID is set as the default id to BaseEntity::m_EntityGroup
     - ENTITY_GROUP_MIN_ID is 'half-reserved': User should avoid using ENTITY_GROUP_MIN_ID
	   as some special group. It should be used as some default/general entity group
     - when an invalid name is set, id wil be set to ENTITY_GROUP_INVALID_ID

 - when a copy entity is created, its entity group is determined either one of the following 3 versions
   - by BaseEntity::m_EntityGroup
     - ENTITY_GROUP_MIN_ID is set as the default id
   - by CCopyEntityDesc::sGroupID
     - overwrites BaseEntity::m_EntityGroup
	 - ENTITY_GROUP_INVALID_ID is set as the default id
   - in BaseEntity::InitCopyEntity()
     - manually set in InitCopyEntity(). overwrites next 2 settings

 - a copy entity with ENTITY_GROUP_INVALID_ID does not collide with any other entity
*/
class EntityGroupHandle : public IArchiveObjectBase
{
	int m_ID;

	std::string m_EntityGroupName;

private:

public:

	EntityGroupHandle() : m_ID(ENTITY_GROUP_INVALID_ID) {}

//	inline EntityGroupHandle( const EntityGroupHandle& handle );

	~EntityGroupHandle() {}

	inline const std::string& GetGroupName() const { return m_EntityGroupName; }

	inline void SetGroupName( const std::string& entity_group_name );

	inline void SetID( int id );

	inline int GetID() const { return m_ID; }

	inline virtual void Serialize( IArchive& ar, const unsigned int version );

	friend class BaseEntity;
};


inline void EntityGroupHandle::Serialize( IArchive& ar, const unsigned int version )
{
	std::string strTemp;
	if( ar.GetMode() == IArchive::MODE_INPUT )
	{
		// restore the technique name from the archive
		ar & strTemp;
		SetGroupName( strTemp.c_str() );
	}
	else
	{
		// record the technique name to the archive
		strTemp = GetGroupName();
		ar & strTemp;
	}
}


//============================= inline implementations =============================

/*
inline EntityGroupHandle::EntityGroupHandle( const EntityGroupHandle& handle )
{
	m_pcName = NULL;
	m_Index = UNINITIALIZED;
    memset( m_acName, '\0', sizeof(m_acName) );

	SetTechniqueName( handle.GetTechniqueName() );
}

inline const char *EntityGroupHandle::GetTechniqueName() const
{
	if( m_pcName )
		return m_pcName;
	else
		return m_acName;
}
*/

inline void EntityGroupHandle::SetGroupName( const std::string& entity_group_name )
{
	m_EntityGroupName = entity_group_name;

	// the previous technique index is invalid
	// - mark it as uninitialized
	m_ID = ENTITY_GROUP_ID_UNINITIALIZED;
}


inline void EntityGroupHandle::SetID( int id )
{
	m_ID = id;

	// clear the name
	m_EntityGroupName = "";
}

} // namespace amorphous



#endif		/*  __EntityGroupHandle_H__  */
