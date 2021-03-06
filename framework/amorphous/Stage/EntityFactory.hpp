#ifndef  __EntityFactory_H__
#define  __EntityFactory_H__


#include <vector>
#include <string>

#include "amorphous/Support/shared_prealloc_pool.hpp"
#include "fwd.hpp"


namespace amorphous
{


class EntityFactory
{
	shared_prealloc_pool<CCopyEntity> m_CopyEntityPool;

	shared_prealloc_pool<AlphaEntity> m_AlphaEntityPool;

	shared_prealloc_pool<LightEntity> m_LightEntityPool;

	shared_prealloc_pool<SoundEntity> m_SoundEntityPool;

	shared_prealloc_pool<ScriptedCameraEntity> m_ScriptedCameraEntityPool;

public:

	enum eEntityFactoryParams
	{
		DEFAULT_MAX_NUM_ENTITIES = 1024,
		DEFAULT_MAX_NUM_ALPHA_ENTITIES = 64,
		DEFAULT_MAX_NUM_LIGHT_ENTITIES = 64,
		DEFAULT_MAX_NUM_SOUND_ENTITIES = 128,
		DEFAULT_MAX_NUM_SCRIPTED_CAMERA_ENTITIES = 16,
	};

public:

	EntityFactory();

	virtual ~EntityFactory();

	/// allocate memory for entities
	/// must be called from a derived class if overridden
	virtual void Init();

	/// \param entity_type_id determines the type of entity to be created.
	/// By default, always returns CCopyEntity()
	virtual std::shared_ptr<CCopyEntity> CreateEntity( unsigned int entity_type_id );

	/// must be implemented in a derived class to create user defined entity
	virtual std::shared_ptr<CCopyEntity> CreateDerivedEntity( unsigned int entity_type_id ) { return std::shared_ptr<CCopyEntity>(); }

	void ReleaseEntity( std::shared_ptr<CCopyEntity> pEntity );

	/// must be implemented in a derived class if user defined entity is used
	/// - user needs to downcast 'pEntity' to release a derived entity
	virtual void ReleaseDerivedEntity( std::shared_ptr<CCopyEntity> pEntity ) {}

	void ReleaseAllEntities();

	/// must be implemented in a derived class if user defined entity is used
	virtual void ReleaseAllDerivedEntities() {}

};


//
// template of a derived entity factory class
//

/*
#ifndef  __DerivedEntityFactory_H__
#define  __DerivedEntityFactory_H__

#include "EntityFactory.hpp"

class CDerivedEntityFactory : public EntityFactory
{
	fixed_prealloc_pool<CDerivedEntity00> m_DerivedEntityPool00;
	fixed_prealloc_pool<CDerivedEntity01> m_DerivedEntityPool01;
	fixed_prealloc_pool<CDerivedEntity02> m_DerivedEntityPool02;

public:

	enum eDerivedEntityFactoryParams
	{
		DEFAULT_MAX_NUM_USER_DEFINED_ENTITIES = 256,
	};

public:

	CDerivedEntityFactory() {}

	virtual ~CDerivedEntityFactory() {}

	virtual CCopyEntity *CreateDerivedEntity( unsigned int entity_type_id );

	virtual void ReleaseDerivedEntity( CCopyEntity *pEntity );

	virtual void ReleaseAllDerivedEntities();
};

class CUserDerivedBaseEntityBase : public CBE_PhysicsBaseEntity
{
public:

	...

}

class CDerivedEntity : public CCopyEntity
{
public:

	enum eUserDerivedEntityID
	{
		DERIVED_ID0 = BaseEntity::USER_DEFINED_ENTITY_ID_OFFSET,
		DERIVED_ID1,
		DERIVED_ID2,
		NUM_DERIVED
	};
};


class CUserDerivedBaseEntity0 : public CBE_PhysicsBaseEntity
{
public:

	...

	virtual unsigned int GetArchiveObjectID() { return DERIVED_ID0; }
}

class CUserDerivedBaseEntity1 : public CBE_PhysicsBaseEntity
{
public:

	...

	virtual unsigned int GetArchiveObjectID() { return DERIVED_ID1; }
}
#endif		/  __DerivedEntityFactory_H__  /

/// cpp

#include "DerivedEntityFactory.hpp"

CCopyEntity *CDerivedEntityFactory::CreateDerivedEntity( unsigned int entity_type_id )
{
	switch( entity_type_id )
	{
	case USER_DEFINED_ENTITY_00:
	case USER_DEFINED_ENTITY_01:
	case USER_DEFINED_ENTITY_02:
		return m_DerivedEntityPool.get();

	default:
		return NULL;
	}

	return NULL;
}

void CDerivedEntityFactory::ReleaseDerivedEntity( CCopyEntity *pEntity )
{
	switch( base_entity_type_id )
	{
	case USER_DEFINED_ENTITY_00:
	case USER_DEFINED_ENTITY_01:
	case USER_DEFINED_ENTITY_02:
		return m_DerivedEntityPool.release( (CDerivedEntity *)pEntity );

	default:
		return;
	}
}

void CDerivedEntityFactory::ReleaseAllDerivedEntities()
{
	EntityFactory::ReleaseAllEntities();

	m_DerivedEntityPool.release_all();
}

*/

} // namespace amorphous



#endif		/*  __EntityFactory_H__  */
