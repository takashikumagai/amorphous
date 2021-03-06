#include "CopyEntity.hpp"
#include "BaseEntity.hpp"
//#include "EntitySet.hpp"
#include "amorphous/Physics/Actor.hpp"


namespace amorphous
{
using namespace physics;

using std::shared_ptr;


void CCopyEntity::ApplyWorldImpulse( const Vector3& vImpulse, const Vector3& vContactPoint )
{
/*	if( !pPhysicsActor )
		return;	// entity must have a corresponding physics actor to apply impulse

//	pPhysicsActor->ApplyWorldImpulse( vImpulse, vContactPoint );
	pPhysicsActor->AddWorldForceAtWorldPos( vImpulse, vContactPoint, ForceMode::Impulse );
*/

	for( size_t i=0; i<m_vecpPhysicsActor.size(); i++ )
	{
		m_vecpPhysicsActor[i]->AddWorldForceAtWorldPos( vImpulse, vContactPoint, ForceMode::Impulse );
	}
}


// called during the termination if a copy entity has any parent or children
void CCopyEntity::DisconnectFromParentAndChildren()
{
	// disconnect from children
	int i;
	for( i=0; i<iNumChildren; i++ )
	{
		shared_ptr<CCopyEntity> pChild = m_aChild[i].Get();
		if( pChild )
		{
			pChild->m_pParent = NULL;
			m_aChild[i].Reset();
		}
//		m_aChild[i] = NULL;
	}
	iNumChildren = 0;

	// disconnect from parent
	if( GetParent() )
	{
		// search myself in the parent's list of children
		for( i=0; i< m_pParent->iNumChildren; i++ )
		{
			if( m_pParent->m_aChild[i].GetRawPtr() == this )
			{
				m_pParent->m_aChild[i].Reset();
				int j;
				// push the parent's children forward to fill the vacant space in the array
				for( j=i; j<m_pParent->iNumChildren-1; j++ )
					m_pParent->m_aChild[j] = m_pParent->m_aChild[j+1];

				// set null entity handle to the unused entry
				// - not a required operation. Just to make things clear
				m_pParent->m_aChild[ m_pParent->iNumChildren-1 ] = EntityHandle<>();

				m_pParent->iNumChildren--;

				break;
			}
		}
	}
	m_pParent = NULL;
}


void CCopyEntity::ReleasePhysicsActor()
{
//	pPhysicsActor = NULL;

	for( size_t i=0; i<m_vecpPhysicsActor.size(); i++ )
	{
		m_vecpPhysicsActor[i] = NULL;
	}
	m_vecpPhysicsActor.resize( 0 );

	for( size_t i=0; i<m_vecpPhysicsJoint.size(); i++ )
	{
		m_vecpPhysicsJoint[i] = NULL;
	}
	m_vecpPhysicsJoint.resize( 0 );

}


// This should be ApplyPhysSimResults(), or some name like that.
void CCopyEntity::UpdatePhysics()
{
/*
	pPhysicsActor->GetWorldPose( WorldPose );
*/
	if( m_vecpPhysicsActor.size() == 0 )
		return;

	physics::CActor *pPhysActor = GetPrimaryPhysicsActor();
	if( !pPhysActor )
		return;

//	if( pPhysActor->IsStatic() )
	if( pPhysActor->GetBodyFlags() & PhysBodyFlag::Static
	 || pPhysActor->GetBodyFlags() & PhysBodyFlag::Kinematic )
	{
		return;
	}

	// update the world pose of the entity
	// - copy the world pose of the primary physics actor to that of the entity
	pPhysActor->GetWorldPose( m_WorldPose );

//	for( size_t i=0; i<m_vecpPhysicsActor.size(); i++ )
//		m_vecpPhysicsActor[i]->GetWorldPose( WorldPose );

	int i;
	for( i=0; i<9; i++ )
	{
		if( m_WorldPose.matOrient.GetData()[i] != 0.0f )
			break;
	}
	if( i == 9 )
		int error = 1;
/*
//	Position() = pPhysicsActor->GetPosition();
	Velocity() = pPhysicsActor->GetLinearVelocity();
	AngularVelocity() = pPhysicsActor->GetAngularVelocity();
*/

	Velocity()        = m_vecpPhysicsActor[0]->GetLinearVelocity();
	AngularVelocity() = m_vecpPhysicsActor[0]->GetAngularVelocity();
}


} // namespace amorphous
