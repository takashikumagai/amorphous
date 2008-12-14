#include "EntityFactory.h"
#include "CopyEntity.h"
#include "CopyEntityDesc.h"

#include "Support/memory_helpers.h"

using namespace std;
using namespace boost;


CEntityFactory::CEntityFactory()
{
	Init();
}


CEntityFactory::~CEntityFactory()
{
}


void CEntityFactory::Init()
{
	m_CopyEntityPool.init( DEFAULT_MAX_NUM_ENTITIES );
	m_AlphaEntityPool.init( DEFAULT_MAX_NUM_ALPHA_ENTITIES );
	m_LightEntityPool.init( DEFAULT_MAX_NUM_LIGHT_ENTITIES );
}


shared_ptr<CCopyEntity> CEntityFactory::CreateEntity( unsigned int entity_type_id )
{
	switch( entity_type_id )
	{
	case CCopyEntityTypeID::DEFAULT:
		return m_CopyEntityPool.get_new_object();
	case CCopyEntityTypeID::ALPHA_ENTITY:
		return m_AlphaEntityPool.get_new_object();
	case CCopyEntityTypeID::LIGHT_ENTITY:
	{
		shared_ptr<CLightEntity> pLightEntity = m_LightEntityPool.get_new_object();
		pLightEntity->m_pLightEntitySelf = pLightEntity;
		return pLightEntity;
	}

	// if your derived entity does not require memory pool, you can
	// simply allocate them on heap with default 'new' and return
	// the shared pointer of it
	// e.g., because they are created before the stage starts and not likely
	// to impact performance during gameplay
	// CEntitySet needs to set stock id to -2 after this to mark this entity as non-pooled object
//	case CCopyEntityTypeID::ANOTHER_ENTITY:
//		return shared_ptr<CCopyEntity>( new CAnotherEntity );
	default:
		return CreateDerivedEntity( entity_type_id );
	}

	return shared_ptr<CCopyEntity>();
}


void CEntityFactory::ReleaseEntity( shared_ptr<CCopyEntity> pEntity )
{
	switch( pEntity->GetEntityTypeID() )
	{
	case CCopyEntityTypeID::DEFAULT:
		m_CopyEntityPool.release( pEntity );
		break;
	case CCopyEntityTypeID::ALPHA_ENTITY:
		m_AlphaEntityPool.release<CCopyEntity>( pEntity );
//		m_AlphaEntityPool.release( dynamic_pointer_cast<CAlphaEntity,CCopyEntity>(pEntity) );
		break;
//	case CCopyEntityTypeID::LIGHT_ENTITY:
//		???
//		break;
//	case CCopyEntityTypeID::ANOTHER_ENTITY:
//		pEntity.reset();
		break;
	default:
		ReleaseDerivedEntity( pEntity );
		break;
	}
}


void CEntityFactory::ReleaseAllEntities()
{
	m_CopyEntityPool.release_all();

	m_AlphaEntityPool.release_all();

	ReleaseAllDerivedEntities();
}
