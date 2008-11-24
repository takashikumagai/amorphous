#include "BE_Enemy.h"
#include "GameMessage.h"
#include "CopyEntity.h"
#include "CopyEntityDesc.h"
#include "trace.h"
#include "Stage.h"

#include "EntityMotionPathRequest.h"

#include "Stage/PlayerInfo.h"

#include "3DCommon/Direct3D9.h"
#include "3DCommon/D3DXSMeshObject.h"
#include "3DCommon/Shader/Shader.h"
#include "3DCommon/Shader/ShaderManager.h"

#include "GameCommon/BasicGameMath.h"
#include "GameCommon/MTRand.h"
#include "Support/VectorRand.h"

#include "Sound/Serialization_SoundHandle.h"
#include "Sound/SoundManager.h"

#include "Support/memory_helpers.h"
#include "Support/msgbox.h"
#include "Support/Log/DefaultLog.h"
#include "Support/Vec3_StringAux.h"

#include "Physics/Actor.h"

using namespace physics;


//=======================================================================
// SBE_EnemyExtraData
//=======================================================================

SBE_EnemyExtraData::SBE_EnemyExtraData()
{
	fCurrentManeuverTime = 0;
	fTotalManeuverTime   = 0;
	vManeuverDirection   = Vector3(0,0,0);
	vLastCheckedPlayerPosition    = Vector3(0,0,0);
	vLastCheckedDirectionToPlayer = Vector3(0,0,0);
	fLastCheckedSqDistToPlayer    = 0;
	vLastHitFromThisDirection     = Vector3(0,0,0);
	vOriginalPosition             = Vector3(0,0,0);
	vTargetPosition               = Vector3(0,0,0);
	fSensoringInterval2           = 0.2f * rand() / (float)RAND_MAX;
	pPath = NULL;
}


void SBE_EnemyExtraData::Serialize( IArchive& ar, const unsigned int version )
{
	ar & fCurrentManeuverTime;
	ar & fTotalManeuverTime;
	ar & vManeuverDirection;
	ar & vLastCheckedPlayerPosition;
	ar & vLastCheckedDirectionToPlayer;
	ar & fLastCheckedSqDistToPlayer;
	ar & vLastHitFromThisDirection;
	ar & vOriginalPosition;
	ar & vTargetPosition;
	ar & fSensoringInterval2;

	// TODO: serialize motion path
}



//=======================================================================
// CBE_Enemy
//=======================================================================

CBE_Enemy::CBE_Enemy()
{
	m_iExtraDataIndex = 0;

	m_bLighting = true;

	m_AttribFlag = 0;

	int i;
	for( i=0; i<CEnemyState::NUM_STATES; i++ )
		m_apEnemyState[i] = NULL;

	m_apEnemyState[CEnemyState::STATE_SEARCH]      = new CES_Search;
	m_apEnemyState[CEnemyState::STATE_ATTACK]      = new CES_Attack;
	m_apEnemyState[CEnemyState::STATE_UNDERATTACK] = new CES_UnderAttack;

	m_fMobility = 1.0f;
	m_fYawSpeed = 1.0f;
	m_fPitchSpeed = 1.0f;

	m_iNumFragments = 0;

	m_vLocalMuzzlePosition = Vector3( 0.0f, 0.0f, 1.2f );

//	m_ActorDesc.iCollisionGroup = ENTITY_COLL_GROUP_OTHER_ENTITIES;

	// enemy base entity inherits physics base entity.
	// However, its physics simulation is turned off by default so that
	// each specific enemy can decide whether it actually uses physics or not
	ClearEntityFlag( BETYPE_RIGIDBODY );
}


CBE_Enemy::~CBE_Enemy()
{
	int i;
	for( i=0; i<CEnemyState::NUM_STATES; i++ )
	{
		SafeDelete( m_apEnemyState[i] );
	}
}


void CBE_Enemy::Init()
{
	if( strlen(m_Explosion.GetBaseEntityName()) == 0 )
		m_Explosion.SetBaseEntityName( "imex" );

//	if( strlen(m_Explosion.GetBaseEntityName()) == 0 )
//		m_MuzzleFlash.SetBaseEntityName( "mflsh" );

	for( int i=0; i<NUM_BULLET_TYPES; i++ )
		LoadBaseEntity( m_Bullet[i] );

	for( int i=0; i<m_iNumFragments; i++ )
		LoadBaseEntity( m_aFragment[i].BaseEntity );
//		LoadBaseEntity( m_aFragment[i] );

	LoadBaseEntity( m_MuzzleFlash );
	LoadBaseEntity( m_Explosion );
}


void CBE_Enemy::InitCopyEntity(CCopyEntity* pCopyEnt)
{
	pCopyEnt->fLife = m_fLife;
//	pCopyEnt->GroupIndex = CE_GROUP_ENEMY;

	short& rsCurrentState = pCopyEnt->s1;
	rsCurrentState = CEnemyState::STATE_SEARCH;

	float& rfSensoringInterval	= pCopyEnt->f1;
	rfSensoringInterval = 0.2f * rand() / (float)RAND_MAX;

	pCopyEnt->iExtraDataIndex = GetNewExtraDataID();
	SBE_EnemyExtraData *pExtraData = GetExtraData( pCopyEnt->iExtraDataIndex );
	pExtraData->vOriginalPosition = pCopyEnt->Position();
	pExtraData->vTargetPosition   = pCopyEnt->Position();

	if( m_AttribFlag & ATTRIB_OPTCAM )
		pCopyEnt->EntityFlag |= BETYPE_USE_ZSORT;
//		pCopyEnt->bUseZSort = true;
}


void CBE_Enemy::Act(CCopyEntity* pCopyEnt)
{
	SBE_EnemyExtraData* ex = GetExtraData( pCopyEnt->iExtraDataIndex );

	if( ex->pPath )
	{
		const float time_in_stage = (float)m_pStage->GetElapsedTime();

		if( ex->pPath->IsAvailable(time_in_stage) )
		{
			// follow the scripted path
			Matrix34 world_pose = ex->pPath->GetPose( time_in_stage );
			pCopyEnt->SetWorldPose( world_pose );

			// calc velocity - used when the entity is destroyed
			// and the vel of the frags have to be calculated
			float dt = m_pStage->GetFrameTime();
			pCopyEnt->Velocity() = ( pCopyEnt->Position() - pCopyEnt->vPrevPosition ) / dt;
		}
		else
		{
			// done with the scripted motion
			SafeDelete( ex->pPath );
		}
	}
	else
	{
        short& rsCurrentState = pCopyEnt->s1;
		m_apEnemyState[rsCurrentState]->Act( *pCopyEnt, *this, (float)m_pStage->GetElapsedTime() );
	}
}


void CBE_Enemy::SearchPlayer(CCopyEntity* pCopyEnt, short& rsMode,
							  Vector3& rvDesiredDirection, float* pfSqDistToPlayer)
{

	// ========= enable the following 2 lines to use enemy observation mode ==========
//	rsMode = CEnemyState::STATE_SEARCH;
//	return;


	Vector3 vStart, vMyselfToPlayer;
	CCopyEntity* pPlayer = PLAYERINFO.GetCurrentPlayerBaseEntity()->GetPlayerCopyEntity();

	vStart = pCopyEnt->Position() + pCopyEnt->GetDirection() * 1.42f;
	vMyselfToPlayer = pPlayer->Position() - vStart;

	// chehck the distance to the player
	float fSqDist = Vec3LengthSq( vMyselfToPlayer );
	if( pfSqDistToPlayer )
		*pfSqDistToPlayer = fSqDist;
	if( 40000 < fSqDist )
	{	// too far from the player
		rsMode = CEnemyState::STATE_SEARCH;
		return;
	}

	if( !CheckRayToPlayer(pCopyEnt) )
	{
		rsMode = CEnemyState::STATE_SEARCH;	// there is an obstacle between the player
		return;
	}

	// set 'rvDesiredDirection' - unit vector pointing at the target(player)
	Vec3Normalize( vMyselfToPlayer, vMyselfToPlayer );
	rvDesiredDirection = vMyselfToPlayer;

	float fDotProduct = Vec3Dot( pCopyEnt->GetDirection(), vMyselfToPlayer );

	// 'fDotProduct' ranges from -1 through 1
	if( 0 < fDotProduct )
	{	//the player is visible from this entity
		rsMode = CEnemyState::STATE_ATTACK;	// move to attack mode
	}
	else
	{
		if( fSqDist < 400.0f )
			rsMode = CEnemyState::STATE_ATTACK;	// close enough to hear the player
		else
			rsMode = CEnemyState::STATE_SEARCH;
	}
}

/*
void CBE_Enemy::CheckRayToPlayer(CCopyEntity* pCopyEnt)
{
	if( pCopyEnt->sState & CESTATE_ATREST )
		return;

	float fFrametime = m_pStage->GetFrameTime();
	CCopyEntity* pPlayer = PLAYERINFO.GetCurrentPlayerBaseEntity->GetPlayerCopyEntity();
///	CCopyEntity* pPlayer = PlayerShip.GetPlayerCopyEntity();

	if( pCopyEnt->bInSolid )
	{	// continue penetration iteration
		PenetrationIteration( pCopyEnt, fFrametime );
		return;
	}

	// perform line segment check first

	STrace tr;
	Vector3 vStart = pCopyEnt->Position();
	Vector3 vGoal =  pPlayer->Position();
	tr.pvStart = &vStart;
	tr.pvGoal  = &vGoal;
	tr.bvType = BVTYPE_DOT;
	tr.fRadius = 0;
	tr.pSourceEntity = pCopyEnt;
	tr.fFraction = 1.0f;
	tr.sTraceType = TRACETYPE_IGNORE_NOCLIP_ENTITIES;


	this->m_pStage->ClipTrace( tr );

	pCopyEnt->Position() = tr.vEnd;
	pCopyEnt->touch_plane = tr.plane;	// save contacted surface


	if( tr.fFraction < 1.0f )	// hit something
	{
		pCopyEnt->AddTouchedEntity( tr.pTouchedEntity );
		if( !tr.pTouchedEntity )	// hit static geometry
			CreateBulletHoleDecal( tr.vEnd, tr.plane.normal );

		float fFrameTime_Left = fFrametime * (1.0f - tr.fFraction);

		// start pseudo penetration simulation
		return PenetrationIteration( pCopyEnt, fFrameTime_Left );

	}
	else // i.e. tr.fFraction == 1.0f
	{
		return true;
	}

}
*/

#define ERS_NOT_IN_SOLID	1
#define ERS_IN_SOLID		2

#define CBE_ENEMY_RADAR_PENETRATION_CALC_STEP	0.32f

bool CBE_Enemy::CheckRayToPlayer( CCopyEntity* pCopyEnt )
{
	CCopyEntity* pPlayer = PLAYERINFO.GetCurrentPlayerBaseEntity()->GetPlayerCopyEntity();
///	CCopyEntity* pPlayer = PlayerShip.GetPlayerCopyEntity();
	STrace tr;
	Vector3 vCurrentPos = pCopyEnt->Position();
	Vector3 vGoal       = pPlayer->Position();
	tr.pvStart       = &vCurrentPos;
	tr.pvGoal        = &vGoal;
	tr.bvType        = BVTYPE_DOT;
	tr.fRadius       = 0;
	tr.pSourceEntity = pCopyEnt;
	tr.sTraceType    = TRACETYPE_IGNORE_NOCLIP_ENTITIES;
	Vector3 vDirToPlayer = vGoal - vCurrentPos;
	Vec3Normalize( vDirToPlayer, vDirToPlayer );	// normalization
	short sRadarPenetrationCount = 2;
	short sRadarState = ERS_NOT_IN_SOLID;

	while( 0 <= sRadarPenetrationCount )
	{
		if( sRadarState == ERS_NOT_IN_SOLID )
		{	// check trace to the player
//			tr.pvStart = &vCurrentPos;
//			vGoal  =  &pPlayer->Position();
			tr.in_solid = false;
			tr.fFraction = 1.0f;
			this->m_pStage->ClipTrace( tr );

			vCurrentPos = tr.vEnd;

			if( tr.fFraction < 1.0f )	// hit something
			{
				if( tr.pTouchedEntity == pPlayer )
					return true;	// player is in a visible position
				else
				{	// hit static geometry or another entity
					sRadarState = ERS_IN_SOLID;
					vCurrentPos += vDirToPlayer * 0.01f;	// put 'vCurrentPos' in solid region
					sRadarPenetrationCount--;
					continue;
				}
			}
			else // i.e. tr.fFraction == 1.0f
			{	// no hit - an exception. trace should at least hit the player
				return false;
			}
		}
		else // i.e. sRadarState == ERS_IN_SOLID
		{
			vCurrentPos += vDirToPlayer * CBE_ENEMY_RADAR_PENETRATION_CALC_STEP;
			tr.vEnd = vCurrentPos;
			tr.fFraction = 1.0f;
			tr.in_solid = false;
			m_pStage->CheckPosition( tr );

			if( tr.in_solid )
			{	// proceeding inside the material
				sRadarPenetrationCount--;
				continue;
			}
			else
			{	// penetrated through the material

				// move a little forward so that 'vCurrentPos' can safely get out of the wall
				vCurrentPos += vDirToPlayer * 0.05f;
				sRadarState = ERS_NOT_IN_SOLID;
				continue;
			}
		}
	}

	// player is not visible to this enemy entity
	return false;
}


void CBE_Enemy::UpdateDesiredYawAndPitch(CCopyEntity* pCopyEnt, Vector3& rvDesiredDirection)
{
	float& rfDesiredDeltaYaw	= pCopyEnt->f2;	//ideal angle of yaw rotation (relative) 
	float& rfDesiredPitch		= pCopyEnt->f3; //ideal angle of pitch rotation (absolute) 
	Vector3& rvDesiredDirection_H = pCopyEnt->v2;	//horizontal desired direction
	Vector3& rvCurrentDirection_H = pCopyEnt->v3;	//horizontal current direction


	// normalze the unit vector to prevent the problem
	// caused by the error of float value
	Vector3 vDirTemp;
	Vec3Normalize( vDirTemp, pCopyEnt->GetDirection() );
	pCopyEnt->SetDirection( vDirTemp );
	rvCurrentDirection_H = pCopyEnt->GetDirection();
	rvCurrentDirection_H.y = 0;
	Vec3Normalize( rvCurrentDirection_H, rvCurrentDirection_H );

	// now, forward direction and horizontal forward direction have been prepared

	// update horizontal desired direction
	rvDesiredDirection_H = rvDesiredDirection;
	rvDesiredDirection_H.y = 0;
	Vec3Normalize( rvDesiredDirection_H, rvDesiredDirection_H );
		
	// update desired yaw (yaw to cover in order to aim the player)
	float fDotProduct = Vec3Dot( rvCurrentDirection_H, rvDesiredDirection_H );
	if( fabs(1.0 - fDotProduct) < 0.0001 )
		rfDesiredDeltaYaw = 0;
	else
		rfDesiredDeltaYaw = (float)acos(fDotProduct);

	// check the direction of yaw
	Vector3 vDirY;
	D3DXVec3Cross( &vDirY, &rvCurrentDirection_H, &rvDesiredDirection_H );
	if( vDirY.y < 0 )
		rfDesiredDeltaYaw *= (-1);

	// update desired pitch
	rfDesiredPitch = (float)acos( D3DXVec3Dot( &rvDesiredDirection, &rvDesiredDirection_H ) );

	// when the pitch is upward, the rotation angle is negative (in left-hand coordinate)
	if( 0 < rvDesiredDirection.y - pCopyEnt->GetDirection().y )
		rfDesiredPitch *= (-1);
}


void CBE_Enemy::AimAlong(CCopyEntity* pCopyEnt, Vector3& rvDesiredDirection)
{
	if( rvDesiredDirection == Vector3(0,0,0) )
		return;

	float& rfDesiredDeltaYaw	= pCopyEnt->f2;	//ideal angle of yaw rotation (relative) 
	float& rfDesiredPitch		= pCopyEnt->f3; //ideal angle of pitch rotation (absolute) 
	Vector3& rvDesiredDirection_H = pCopyEnt->v2;	//horizontal desired direction
	Vector3& rvCurrentDirection_H = pCopyEnt->v3;	//horizontal current direction

	//========================== Yaw ==========================
	float fDeltaYaw;
	fDeltaYaw = m_fYawSpeed * m_pStage->GetFrameTime();

	if( rfDesiredDeltaYaw < 0 )
		fDeltaYaw *= (-1);

	// do not exceed the desired angle
	if( fabs(rfDesiredDeltaYaw) <= fabs(fDeltaYaw) )
		fDeltaYaw = rfDesiredDeltaYaw;

	// get rotation matrix around the y-axis
	D3DXMATRIX matRot;
	D3DXMatrixRotationY( &matRot, fDeltaYaw );

	// rotate a little toward the player
	Vector3 vDirTemp;
	D3DXVec3TransformCoord( &vDirTemp, &pCopyEnt->GetDirection(), &matRot );
	pCopyEnt->SetDirection( vDirTemp );
	D3DXVec3TransformCoord( &rvCurrentDirection_H, &rvCurrentDirection_H, &matRot );

	// rotate 'vCurrentDirection_H' 90 degrees to update 'right' direction vector
	Vector3 vRight = pCopyEnt->GetRightDirection();
	vRight.x = rvCurrentDirection_H.z;
	vRight.y = 0;	// no bank
	vRight.z =-rvCurrentDirection_H.x;

	rfDesiredDeltaYaw -= fDeltaYaw;

	//========================== Pitch ==========================
	Vector3 vDirection = pCopyEnt->GetDirection();
	float fDeltaPitch, fCurrentPitch, fDesiredDeltaPitch;
	fDeltaPitch = m_fPitchSpeed * m_pStage->GetFrameTime();
	fCurrentPitch = (float)acos( Vec3Dot( rvCurrentDirection_H, vDirection ) );
	if( 0 < vDirection.y )
		fCurrentPitch *= (-1);

	fDesiredDeltaPitch = rfDesiredPitch - fCurrentPitch;
	if( vDirection.y <= rvDesiredDirection.y )
	{	// pitch rotation is upward - rotation angle must be
		// negative in the left hand coordinate
		fDeltaPitch *= (-1);
	}
	if( fabs(fDesiredDeltaPitch) < fabs(fDeltaPitch) )
		fDeltaPitch = fDesiredDeltaPitch;

	fCurrentPitch += fDeltaPitch;

	// pitch must be between -45 and 85 degrees
	if( fCurrentPitch < -1.48f )
		fCurrentPitch = -1.48f;	 // do not look up higher than 85 degrees
	else if( 0.785f < fCurrentPitch )
		fCurrentPitch =  0.785f; // do not look down lower than -45 degrees

	// get rotation matrix around the axis
	D3DXMatrixRotationAxis( &matRot, &vRight, fCurrentPitch );
	D3DXVec3TransformCoord( &vDirection, &rvCurrentDirection_H, &matRot );
	Vector3 vUp;
//	D3DXVec3Cross( &vUp, &pCopyEnt->GetDirection(), &pCopyEnt->GetRightDirection() );
	D3DXVec3Cross( &vUp, &vDirection, &vRight );

	pCopyEnt->SetDirection_Right( vRight );
	pCopyEnt->SetDirection_Up( vUp );
	pCopyEnt->SetDirection( vDirection );
}


void CBE_Enemy::UpdateScriptedMotionPath( CCopyEntity* pCopyEnt, CBEC_MotionPath& rPath )
{
	const float time_in_stage = (float)m_pStage->GetElapsedTime();

	if( rPath.IsAvailable(time_in_stage) )
	{
		// follow the scripted path
		Matrix34 world_pose = rPath.GetPose( time_in_stage );
		pCopyEnt->SetWorldPose( world_pose );
		if( pCopyEnt->pPhysicsActor )
			pCopyEnt->pPhysicsActor->SetWorldPose( world_pose );

		// calc velocity - used when the entity is destroyed
		// and the vel of the frags have to be calculated
		float dt = m_pStage->GetFrameTime();
		pCopyEnt->Velocity() = ( pCopyEnt->Position() - pCopyEnt->vPrevPosition ) / dt;
	}
	else
	{
		// done with the scripted motion
		rPath.ReleaseMotionPath();
	}
}


void CBE_Enemy::FireAtPlayer(CCopyEntity* pCopyEnt)
{
	float& rfFireCycleTime		= pCopyEnt->f4;

	if( rfFireCycleTime <= m_fFireKeepDuration + m_fFireCeaseInterval )
	{	// keep firing for a couple of seconds and then rest for a moment(repeat this)
		//                   0 <= rfFiringCycleTime <= m_fFireKeepDuration : fire
		// m_fFireKeepDuration <  rfFiringCycleTime <= m_fFireKeepDuration + m_fFireCeaseInterval : hold fire
		rfFireCycleTime += m_pStage->GetFrameTime();
		if( rfFireCycleTime <= m_fFireKeepDuration )
		{	// open fire
			Fire( pCopyEnt );
		}
	}
	else
		rfFireCycleTime = 0;
}


void CBE_Enemy::Fire( CCopyEntity* pCopyEnt )
{
	float& rfLastFireTime		= pCopyEnt->f5;

	if( m_fFiringRate <= rfLastFireTime )
	{
		rfLastFireTime = 0;

		Vector3 vMuzzleVelocity = pCopyEnt->GetDirection() * m_fBulletSpeed;

		Vector3 vWorldMuzzlePosition;
		pCopyEnt->GetWorldPose().Transform( vWorldMuzzlePosition, m_vLocalMuzzlePosition );

/*		Vector3 vWorldMuzzlePosition = pCopyEnt->Position() 
			                             + m_vLocalMuzzlePosition.x * pCopyEnt->Right()
										 + m_vLocalMuzzlePosition.y * pCopyEnt->Up()
										 + m_vLocalMuzzlePosition.z * pCopyEnt->GetDirection();
*/
		CCopyEntityDesc bullet_entity;

		// projectile fired by the enemy is marked as 'CE_GROUP_ID_ENEMYSIDE'
		// this is used by homing missile to differentiate targets
		bullet_entity.sGroupID = GetEntityGroupID( m_ProjectileEntityGroup );//CE_GROUP_ENEMY;

		Vector3 vRand;
		float fRand        = m_fRandomFireDirection;
		float fBulletPower = m_fBulletPower;
		int i, iNumPellets = 1;	// right now, enemy entity can fire only one bullet per shot
		for(i=0; i<iNumPellets; i++)
		{
			// add some randomness to the bullet
			vRand.x = ( 0.5f - (float)rand() / (float)RAND_MAX ) * fRand;
			vRand.y = ( 0.5f - (float)rand() / (float)RAND_MAX ) * fRand;
			vRand.z = ( 0.5f - (float)rand() / (float)RAND_MAX ) * fRand;
			vMuzzleVelocity += vRand;


			bullet_entity.pBaseEntityHandle = &m_Bullet[0];
			bullet_entity.SetWorldPosition( vWorldMuzzlePosition );
			bullet_entity.SetWorldOrient( pCopyEnt->GetWorldPose().matOrient );
			bullet_entity.vVelocity   = vMuzzleVelocity;

//			bullet_entity.f2          = fBulletPower;
			m_pStage->CreateEntity( bullet_entity );

		}

		if( 0 < strlen(m_MuzzleFlash.GetBaseEntityName()) )
		{
			CCopyEntityDesc& rMuzzleFlash = bullet_entity;
			rMuzzleFlash.pBaseEntityHandle = &m_MuzzleFlash;
			rMuzzleFlash.vVelocity = pCopyEnt->Velocity() * 0.9f;
			m_pStage->CreateEntity( rMuzzleFlash );
		}

		// play firing sound of this enemy entity
		SoundManager().PlayAt( m_FireSound, vWorldMuzzlePosition );

	}
	else
		rfLastFireTime += m_pStage->GetFrameTime();
}

void CBE_Enemy::Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other)
{
	if( !pCopyEnt_Other )
		return;

/*	if( 1600.0f < D3DXVec3LengthSq( &pCopyEnt_Other->vVelocity ) )
	{	// hit by something faster than 40.0 m/s
		SBE_EnemyExtraData* pExtraData = GetExtraData( pCopyEnt_Self->iExtraDataIndex );
		D3DXVec3Normalize( &pExtraData->vLastHitFromThisDirection, &pCopyEnt_Other->vVelocity);
	}*/

}


void CBE_Enemy::OnDestroyed( CCopyEntity* pCopyEnt )
{
	// explosion - usu. done by texture animation
	CCopyEntityDesc explosion;
	explosion.pBaseEntityHandle = &m_Explosion;
	explosion.SetWorldPose( pCopyEnt->GetWorldPose() );
	explosion.vVelocity  = Vector3(0,0,0);

	this->m_pStage->CreateEntity( explosion );

	// avoid overlaps with fragment entities
	pCopyEnt->bNoClip = true;

	g_Log.Print( "An enemy entity destroyed - vel%s", to_string(pCopyEnt->Velocity()).c_str() );

	CCopyEntity* pFragEntity;
//			MsgBoxFmt( "%s destroyed - creating %d fragments", pCopyEnt->GetName().c_str(), m_iNumFragments );
	const int num_fragments = m_iNumFragments;
	for(int i=0; i<num_fragments; i++)
	{
		const Fragment& frag = m_aFragment[i];

		CCopyEntityDesc& rFrag = explosion;	// reuse desc object
		rFrag.pBaseEntityHandle = &m_aFragment[i].BaseEntity;

		rFrag.SetWorldPosition( pCopyEnt->GetWorldPose() * frag.vOffset );
//				D3DXVec3TransformCoord( &vWorldFragmentOffset, &m_avFragmentOffset[i], &pCopyEnt->GetOrientation() );
//				rFrag.vPosition  = pCopyEnt->Position() + vWorldFragmentOffset;

		rFrag.SetWorldOrient( pCopyEnt->GetWorldPose().matOrient );

//BSPStageBase	rFrag.vVelocity  = pCopyEnt->Velocity() * 0.3f + vWorldFragmentOffset * 20.0f
//BSPStageBase			         + Vector3( RangedRand(-10.0f,10.0f), RangedRand(-8.0f,8.0f), RangedRand(-10.0f,10.0f) );

		rFrag.vVelocity = Vec3RandDir() * RangedRand( frag.InitVelocity.min, frag.InitVelocity.max )
//		rFrag.vVelocity = Vec3RandDir() * RangedRand( 15.5f, 35.0f )
				        + pCopyEnt->Velocity();

		pFragEntity = this->m_pStage->CreateEntity( rFrag );
		if( pFragEntity && pFragEntity->pPhysicsActor )
			pFragEntity->pPhysicsActor->SetAngularVelocity( Vec3RandDir() * RangedRand( 0.5f, 8.0f ) );
	}

/*
	int num_items = m_vecRewardItem.size();
	float f = 0, rand_val;
	for(i=0; i<m_iNumFragments; i++)
	{
		rand_val = GetRangedRand( 0.0f, 1.0f );
		f += m_vecRewardItem[i].m_fProbability;

		if( rand_val < f )
		{
		}
	}
	*/

	// leave a life item
//			m_pStage->CreateEntity( "s_Life", pCopyEnt->Position(),
//				Vector3(0,0,0), Vector3(0,0,1) );
}


void CBE_Enemy::MessageProcedure(SGameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self)
{
//	int i;
	SBE_EnemyExtraData* ex = GetExtraData( pCopyEnt_Self->iExtraDataIndex );

	float& rfLife = pCopyEnt_Self->fLife;
	short& rsCurrentState = pCopyEnt_Self->s1;

	switch( rGameMessage.iEffect )
	{
	case GM_DAMAGE:
		rfLife -= rGameMessage.fParam1;

		ex->vLastHitFromThisDirection = rGameMessage.pSenderEntity->GetDirection();

		if( rsCurrentState == CEnemyState::STATE_SEARCH )
		{
			rsCurrentState = CEnemyState::STATE_UNDERATTACK;
		}

		SoundManager().PlayAt( "bosu21", pCopyEnt_Self->Position() );

		if( rfLife <= 0 )
		{
			OnDestroyed(pCopyEnt_Self);

			SGameMessage msg( GM_DESTROYED );
			msg.pSenderEntity = pCopyEnt_Self;
			msg.fParam1 = 100.0f;	// score ?
			SendGameMessageTo( msg, PLAYERINFO.GetCurrentPlayerEntity() );
/*
			KillReport rep;
			rep.base_name = GetName();
			rep.entity_name = pCopyEnt_Self->GetName();
			rep.score = ???;
			rep.time = m_pStage->GetElapsedTime();	ms or s ???
			rep.vWorldPos = pCopyEnt_Self->Position();
			rep.kill_type = ;
			rep.type = ;
*/
			// terminate the entity

			// terminate all child entities
			const int num_children = pCopyEnt_Self->GetNumChildren();
			for( int i=0; i<num_children; i++ )
			{
				CCopyEntity *pChild = pCopyEnt_Self->m_aChild[i].GetRawPtr();
				if( !pChild )
					continue;

				pChild->m_pParent = NULL;
				m_pStage->TerminateEntity( pChild );
			}
			pCopyEnt_Self->iNumChildren = 0;

			// terminate myself
			m_pStage->TerminateEntity( pCopyEnt_Self );
		}
		break;

	case GM_SET_MOTION_PATH:
		{
		// set motion path
		if( !ex->pPath )
			ex->pPath = new CBEC_MotionPath;
		EntityMotionPathRequest *pReq = (EntityMotionPathRequest *)rGameMessage.pUserData;
		ex->pPath->SetKeyPoses( pReq->vecKeyPose );
		ex->pPath->SetMotionPathType( pReq->MotionPathType );

//		MsgBoxFmt( "set motion path for enemy entity: %s", pCopyEnt_Self->GetName().c_str() );
		break;
		}
	}
}


void CBE_Enemy::Draw(CCopyEntity *pCopyEnt)
{
	if( !m_MeshProperty.m_MeshObjectHandle.GetMesh() )
		return;
/*
	int shader_tech_id;
	if( m_AttribFlag & ATTRIB_OPTCAM )
		shader_tech_id = SHADER_TECH_DEFAULT;	// SHADER_TECH_ENTITY_OPTCAM;
	else
		shader_tech_id = m_MeshProperty.m_ShaderTechniqueID;	// SHADER_TECH_ENTITY_DEFAULT;

	Draw3DModel( pCopyEnt, shader_tech_id );
*/

	Draw3DModel( pCopyEnt );
}


bool CBE_Enemy::LoadSpecificPropertiesFromFile( CTextFileScanner& scanner )
{
	string sound_name, str_name, group_name;
	float f;

	if( scanner.TryScanLine( "PROJECTILE_GROUP", group_name ) )
	{
		m_ProjectileEntityGroup.SetGroupName( group_name );
		return true;
	}

	if( scanner.TryScanLine( "MOBILITY", m_fMobility ) ) return true;

	if( scanner.TryScanLine( "YAW_SPD", m_fYawSpeed ) ) return true;

	if( scanner.TryScanLine( "PITCH_SPD", m_fPitchSpeed ) ) return true;

	if( scanner.TryScanLine( "BLTNAME", str_name ) )
	{
		m_Bullet[0].SetBaseEntityName( str_name.c_str() );
		return true;
	}

	if( scanner.TryScanLine( "MUZZLE_FLASH", str_name ) )
	{
		m_MuzzleFlash.SetBaseEntityName( str_name.c_str() );
		return true;
	}

	if( scanner.TryScanLine( "BLTPOWER", m_fBulletPower ) ) return true;
	if( scanner.TryScanLine( "BLTSPD", m_fBulletSpeed ) ) return true;

	if( scanner.TryScanLine( "FIRERATE", m_fFiringRate ) ) return true;
	if( scanner.TryScanLine( "FIREKPTIME", m_fFireKeepDuration ) ) return true;
	if( scanner.TryScanLine( "FIREINTVL", m_fFireCeaseInterval ) ) return true;
	if( scanner.TryScanLine( "RANDFIRE", m_fRandomFireDirection ) ) return true;

	if( scanner.TryScanLine( "LOCAL_MUZZLE_POS", m_vLocalMuzzlePosition ) ) return true;

	if( scanner.TryScanLine( "FIRE_SND", sound_name ) )
	{
		m_FireSound.SetResourceName( sound_name );
		return true;
	}

	if( scanner.TryScanLine( "RANDSEARCH", m_iRandomSearchManeuver ) ) return true;

	if( scanner.TryScanLine( "EXPLOSION", str_name ) )
	{
		m_Explosion.SetBaseEntityName( str_name.c_str() );
		return true;
	}

	Vector3 vOffset = Vector3(0,0,0), vRange = Vector3( 15.5f, 35.5f, 0.0f );
	if( scanner.TryScanLine( "FRAGMENT", str_name, vOffset, vRange ) )
//	if( scanner.TryScanLine( "FRAGMENT", str_name, m_avFragmentOffset[m_iNumFragments] ) )
	{
		m_aFragment[m_iNumFragments].BaseEntity.SetBaseEntityName( str_name.c_str() );
		m_aFragment[m_iNumFragments].vOffset = vOffset;
		m_aFragment[m_iNumFragments].InitVelocity.min = vRange.x;
		m_aFragment[m_iNumFragments].InitVelocity.max = vRange.y;
		m_iNumFragments++;
//		m_aFragment[m_iNumFragments++].SetBaseEntityName( str_name.c_str() );
		return true;
	}

	if( scanner.TryScanLine( "WRECK", str_name ) )
	{
		m_Wreck.SetBaseEntityName( str_name.c_str() );
		return true;
	}

	if( scanner.TryScanLine( "REWARDITEM", f, str_name ) )
	{
		m_vecRewardItem.push_back( CRewardItem() );
		m_vecRewardItem.back().m_fProbability = f;
		m_vecRewardItem.back().m_ItemEntity.SetBaseEntityName( str_name.c_str() );
		return true;
	}


//	if( scanner.TryScanLine( "OPTCAM", m_AttribFlag ) ) return true;

	return false;
}


void CBE_Enemy::Serialize( IArchive& ar, const unsigned int version )
{
	CBaseEntity::Serialize( ar, version );

	ar & m_ProjectileEntityGroup;

	ar & m_AttribFlag;

	ar & m_fMobility;
	ar & m_fYawSpeed;
	ar & m_fPitchSpeed;

	ar & m_Bullet[0] & m_Bullet[1];
	ar & m_fBulletPower;
	ar & m_fBulletSpeed;
	ar & m_fFiringRate;	// sec per shot
	ar & m_fFireKeepDuration;
	ar & m_fFireCeaseInterval;
	ar & m_fRandomFireDirection;
	ar & m_FireSound;

	ar & m_vLocalMuzzlePosition;

	ar & m_MuzzleFlash;

	ar & m_iRandomSearchManeuver;

	ar & m_Explosion;
	ar & m_Wreck;

	int i;
	for(i=0;i<NUM_MAX_FRAGMENTS;i++) ar & m_aFragment[i];
//	for(i=0;i<NUM_MAX_FRAGMENTS;i++) ar & m_aFragment[i];
//	for(i=0;i<NUM_MAX_FRAGMENTS;i++) ar & m_avFragmentOffset[i];

	ar & m_iNumFragments;

	ar & m_vecRewardItem;
}
