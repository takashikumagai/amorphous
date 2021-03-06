#include "EntityRenderManager.hpp"
#include "EntitySet.hpp"
#include "MiscEntityRenderers.hpp"
#include "LightEntity.hpp"
#include "trace.hpp"
#include "ViewFrustumTest.hpp"
//#include "RenderContext.hpp"
#include "BE_Skybox.hpp"

#include "amorphous/Graphics/ShadowMapManager.hpp"
#include "amorphous/Graphics/VarianceShadowMapManager.hpp"
#include "amorphous/Graphics/CubeMapManager.hpp"
#include "amorphous/Graphics/Shader/ShaderManagerHub.hpp"
#include "amorphous/Graphics/Shader/GenericShaderGenerator.hpp"
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"
#include "amorphous/Graphics/FogParams.hpp"

#include "amorphous/Graphics/RenderTask.hpp"
#include "amorphous/Graphics/RenderTaskProcessor.hpp"

#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"
#include "amorphous/Support/Vec3_StringAux.hpp"
#include "amorphous/Support/Macro.h"
#include "amorphous/Support/ParamLoader.hpp"

#include "ScreenEffectManager.hpp"


namespace amorphous
{

using namespace std;


bool IsValidPlane( const Plane& plane )
{
	if( fabs(plane.normal.x) < 0.001
	 && fabs(plane.normal.y) < 0.001
	 && fabs(plane.normal.z) < 0.001 )
	{
		return false;
	}
	else
	{
		return true;
	}
}


class PlanarReflectionGroup
{
	SPlane m_Plane;

	std::vector< EntityHandle<> > m_Entities;

	std::shared_ptr<TextureRenderTarget> m_pReflectionRenderTarget;

	bool m_TextureUpdated;

	int FindEntity( EntityHandle<>& entity )
	{
		std::shared_ptr<CCopyEntity> pEntity = entity.Get();
		if( !pEntity )
			return -1;

		for( size_t i=0; i<m_Entities.size(); i++ )
		{
			std::shared_ptr<CCopyEntity> pOtherEntity = m_Entities[i].Get();
			if( pOtherEntity
			 && pOtherEntity->GetID() == pEntity->GetID() )
			{
				return (int)i;
			}
		}

		return -1;
	}

public:

	PlanarReflectionGroup()
		:
	m_Plane( SPlane(Vector3(0,1,0),0) ),
	m_TextureUpdated(false)
	{}

	void Init()
	{
		m_pReflectionRenderTarget = TextureRenderTarget::Create();
		bool res = m_pReflectionRenderTarget->InitScreenSizeRenderTarget();
	}

	void Release()
	{
		m_pReflectionRenderTarget.reset();
	}

	void AddEntity( EntityHandle<>& entity )
	{
		int entity_index = FindEntity( entity );
		if( entity_index == -1 )
			m_Entities.push_back( entity );
		else
		{
			// already registered
			return;
		}
	}

	Result::Name RemoveEntity( EntityHandle<>& entity )
	{
		int entity_index = FindEntity( entity );
		if( 0 <= entity_index && entity_index < (int)m_Entities.size() )
		{
			m_Entities.erase( m_Entities.begin() + entity_index );
			return Result::SUCCESS;
		}
		else
			return Result::INVALID_ARGS;
	}

	friend class EntityRenderManager;
};

std::vector<PlanarReflectionGroup> s_PlanarReflectionGroups;



class CubeTextureParamsLoader : public ShaderParamsLoader
{
	int m_CubeTexIndex;
//	LPDIRECT3DCUBETEXTURE9 m_pCubeTexture;

	EntityHandle<> m_Entity; ///< envmap target

	EntityRenderManager *m_pEntityRenderManager;

public:

	CubeTextureParamsLoader( std::shared_ptr<CCopyEntity> pEntity = std::shared_ptr<CCopyEntity>(),
		                      EntityRenderManager *pEntityRenderMgr = NULL,
							  int cube_tex_index = 0 )
		:
	m_Entity(pEntity),
	m_pEntityRenderManager(pEntityRenderMgr),
	m_CubeTexIndex(cube_tex_index)
//	m_pCubeTexture(pCubeTexture)
	{}

/*	void SetCubeTexture( int cube_tex_index, LPDIRECT3DCUBETEXTURE9 pCubeTexture )
	{
		m_CubeTexIndex = cube_tex_index;
		m_pCubeTexture = pCubeTexture;
	}*/

	void UpdateShaderParams( ShaderManager& rShaderMgr )
	{
//		rShaderMgr.SetCubeTexture( m_CubeTexIndex, m_pCubeTexture );
		std::shared_ptr<CCopyEntity> pEntity = m_Entity.Get();
		if( pEntity && m_pEntityRenderManager )
		{
			TextureHandle cube_map_texture
				= m_pEntityRenderManager->GetEnvMapTexture(pEntity->GetID());

			LOG_PRINT_ERROR( " ShaderManager::SetCubeTexture() has not been implemented yet." );

			rShaderMgr.SetCubeTexture( m_CubeTexIndex, cube_map_texture );
		}
	}
};


class CEntityShadowMapRenderer : public ShadowMapSceneRenderer
{
	EntityRenderManager *m_pRenderer;

public:

	CEntityShadowMapRenderer(EntityRenderManager *pRenderer)
		:
	m_pRenderer(pRenderer)
	{}

	void RenderSceneToShadowMap( Camera& camera, ShaderHandle *shaders, ShaderTechniqueHandle *shader_techniques );

	void RenderShadowReceivers( Camera& camera, ShaderHandle *shaders, ShaderTechniqueHandle *shader_techniques );
};


void CEntityShadowMapRenderer::RenderSceneToShadowMap( Camera& camera, ShaderHandle *shaders, ShaderTechniqueHandle *shader_techniques )
{
	m_pRenderer->RenderShadowCasters( camera );
}


void CEntityShadowMapRenderer::RenderShadowReceivers( Camera& camera, ShaderHandle *shaders, ShaderTechniqueHandle *shader_techniques )
{
	m_pRenderer->RenderShadowReceivers( camera );
}


class CEntityEnvMapRenderTask : public CRenderTask
{
	EntityRenderManager *m_pRenderManager;

public:

	CEntityEnvMapRenderTask( EntityRenderManager *pRenderMgr )
		:
	m_pRenderManager(pRenderMgr)
	{
		m_TypeFlags |= DO_NOT_CALL_BEGINSCENE_AND_ENDSCENE;
	}

	virtual void Render()
	{
		m_pRenderManager->UpdateEnvironmentMapTextures();
	}
};


class CEntityShadowMapRenderTask : public CRenderTask
{
	EntityRenderManager *m_pRenderManager;

	Camera *m_pCamera;

//	ScreenEffectManager *m_pScreenEffectManager;

public:

	CEntityShadowMapRenderTask( EntityRenderManager *pRenderMgr,
		Camera *pCam,
		ScreenEffectManager *pScreenEffectManager)
		:
	m_pRenderManager(pRenderMgr),
	m_pCamera(pCam)//,
//	m_pScreenEffectManager(pScreenEffectManager)
	{
		m_TypeFlags |= DO_NOT_CALL_BEGINSCENE_AND_ENDSCENE;
	}

	virtual void Render()
	{
		GetShaderManagerHub().PushViewAndProjectionMatrices( *m_pCamera );

		// - creates shadow map
		// - creates scene depth map
		// - creates scene texture
		// each of the three has BeginScene() and EndScene() pair inside
		m_pRenderManager->RenderSceneWithShadows( *m_pCamera/*, m_pScreenEffectManager*/ );

		GetShaderManagerHub().PopViewAndProjectionMatrices();
	}
};



class CEntitySceneRenderTask : public CRenderTask
{
	EntityRenderManager *m_pRenderManager;

	Camera *m_pCamera;

public:

	CEntitySceneRenderTask( EntityRenderManager *pRenderMgr, Camera *pCam )
		:
	m_pRenderManager(pRenderMgr),
	m_pCamera(pCam)
	{
	}

	virtual ~CEntitySceneRenderTask() {}

	void Render()
	{
		m_pRenderManager->Render( *m_pCamera );
	}
};


string EntityRenderManager::ms_DefaultFallbackShaderFilename = "Shader\\Default.fx";


EntityRenderManager::EntityRenderManager( EntityManager* pEntitySet )
:
m_pEntitySet(pEntitySet),
m_pCubeMapManager(NULL),
m_pShadowManager(NULL),
m_pCurrentCamera(NULL),
m_IsRenderingMirroredScene(false),
m_CurrentlyRenderedPlanarReflectionSceneID(-1)
{
	// clear z-sort table
	for(int i=0; i<SIZE_ZSORTTABLE; i++)
	{
		m_apZSortTable[i] = NULL;
	}

	m_fCameraFarClipDist = 1000.0f;


	m_NumMaxLightsForShadow = 1;

	// environment mapping - turned off by default
	m_bEnableEnvironmentMap = false;

	m_CurrentEnvMapTargetEntityID = 0;

	// shadow-related settings
	
	m_bOverrideShadowMapLight = false;

	m_vOverrideShadowMapPosition = Vector3(0,0,0);
	m_vOverrideShadowMapDirection = Vector3(0,-1,0);

	m_pShadowMapSceneRenderer.reset( new CEntityShadowMapRenderer(this) );

	LoadFallbackShader();

	s_PlanarReflectionGroups.resize( 0 );
}


EntityRenderManager::~EntityRenderManager()
{
//	ReleaseGraphicsResources();

	m_vecpSweepRenderBaseEntity.resize( 0 );

	m_EntityNodeRendered.clear();

	SafeDelete( m_pShadowManager );

	SafeDelete( m_pCubeMapManager );
}


bool EntityRenderManager::LoadFallbackShader()
{
//	m_FallbackShaderFilepath = ms_DefaultFallbackShaderFilename;
//	if( !m_FallbackShader.Load( m_FallbackShaderFilepath ) )
//	{
//		return false;
//	}

	ShaderResourceDesc desc;
	GenericShaderDesc shader_desc;
	shader_desc.Specular = SpecularSource::UNIFORM;
	desc.pShaderGenerator.reset( new GenericShaderGenerator(shader_desc) );
	bool loaded = m_FallbackShader.Load( desc );

	if( !loaded )
	{
		LOG_PRINT_WARNING( " Failed to load the fallback shader." );
	}

	return true;
}


void EntityRenderManager::UpdateEntityTree( EntityNode* pRootNode, int num_nodes )
{
	m_paEntityTree = pRootNode;
	m_NumEntityNodes = num_nodes;

	m_EntityNodeRendered.clear();
	m_EntityNodeRendered.resize( m_NumEntityNodes, 0 );
}


void EntityRenderManager::AddSweepRenderEntity( BaseEntity* pBaseEntity )
{
	m_vecpSweepRenderBaseEntity.push_back( pBaseEntity );
}


void EntityRenderManager::RenderEntityNodeUp_r( short sEntNodeIndex, Camera& rCam )
{
	EntityNode* pFirstEntNode = m_paEntityTree;
	EntityNode* pEntNode = pFirstEntNode + sEntNodeIndex;

	StandardEntityRenderer entity_renderer( this );
	pEntNode->RenderEntities( entity_renderer, rCam );
	m_EntityNodeRendered[sEntNodeIndex] = 1;	// mark this entity node as an already rendered one

	if( pEntNode->sParent != -1 && m_EntityNodeRendered[ pEntNode->sParent ] != 1)
		RenderEntityNodeUp_r( pEntNode->sParent, rCam);
}


/**
 * put the skybox to the head of the root entity node
 * - assumes that there is only one skybox entity
 */
void EntityRenderManager::MoveSkyboxToListHead()
{
	EntityNode& rRootNode = m_paEntityTree[0];

	CCopyEntity *pEntity = NULL, *pPrevEntity;
	CCopyEntity *pSkyboxEntity = NULL;
	CLinkNode<CCopyEntity> *pLinkNode;
	for( pLinkNode = rRootNode.m_EntityLinkHead.pNext, pPrevEntity = NULL;
		 pLinkNode;
		 pLinkNode = pLinkNode->pNext, pPrevEntity = pEntity )
	{
		pEntity = pLinkNode->pOwner;

		if( pEntity->pBaseEntity->GetArchiveObjectID() == BaseEntity::BE_SKYBOX )
		{
			if( pEntity == rRootNode.m_EntityLinkHead.pNext->pOwner )
				break;	// already placed at the head

			pSkyboxEntity = pEntity;

			pSkyboxEntity->m_EntityLink.Unlink();

			rRootNode.m_EntityLinkHead.InsertNext( &pSkyboxEntity->m_EntityLink );

			break;
		}
	}
}


void EntityRenderManager::RenderZSortTable()
{
	int i;
	CCopyEntity* pEntity;

	// start rendering from distant entities and end with nearby entities
	for( i=SIZE_ZSORTTABLE-1; 0<=i; i-- )
	{
		if( !m_apZSortTable[i] )
			continue;
		
		pEntity = m_apZSortTable[i];

		while(pEntity)
		{
/*			if( pEntity->Lighting() )
			{
				if( pEntity->sState & CESTATE_LIGHT_INFORMATION_INVALID )
				{	// need to update light information - find lights that matter to this entity
					pEntity->ClearLightIndices();
					m_pEntitySet->UpdateLightInfo( pEntity );
					pEntity->sState &= ~CESTATE_LIGHT_INFORMATION_INVALID;
				}

				// turn on lights that reach 'pCopyEnt'
				m_pEntitySet->EnableLightForEntity();
				m_pEntitySet->SetLightsForEntity( pEntity );
			}
			else
			{	// turn off lights
				m_pEntitySet->DisableLightForEntity();
			}
*/
			// render the entity
			pEntity->Draw();

			pEntity = pEntity->m_pNextEntityInZSortTable;
		}
	}
}


void EntityRenderManager::ClearZSortTable()
{
	for( int i=0; i<SIZE_ZSORTTABLE; i++ )
		m_apZSortTable[i] = NULL;
}


void EntityRenderManager::SendToZSortTable(CCopyEntity* pCopyEnt)
{
	float fZinCameraSpace;
	int iZSortValue;

	const float fMaxRenderDepth = m_fCameraFarClipDist;

	const Vector3 vCameraForwardDirection	= m_CameraPose.matOrient.GetColumn(2);//Vector3( m_matCamera._13, m_matCamera._23, m_matCamera._33 );
	const Vector3 vCameraToEntity = pCopyEnt->GetWorldPosition() - m_CameraPose.vPosition;

	fZinCameraSpace = Vec3Dot( vCameraToEntity, vCameraForwardDirection );

	if( fZinCameraSpace < -pCopyEnt->fRadius || fMaxRenderDepth + pCopyEnt->fRadius <= fZinCameraSpace )
		return;	//not in the camera's view frustum

	if( fZinCameraSpace < 0 )
		fZinCameraSpace = 0.0f;
	else if( fMaxRenderDepth <= fZinCameraSpace )
		fZinCameraSpace = fMaxRenderDepth - 0.1f;

	pCopyEnt->fZinCameraSpace = fZinCameraSpace;

	iZSortValue = (int)( fZinCameraSpace / fMaxRenderDepth * (float)(SIZE_ZSORTTABLE - 1) );

	// simple but inaccurate
	//pCopyEnt->m_pNextEntityInZSortTable = m_apZSortTable[iZSortValue];
	//m_apZSortTable[iZSortValue] = pCopyEnt;

	// more accurate but slower
	CCopyEntity *pZSortedEntity, *pPrevZSortedEntity;
	if( !m_apZSortTable[iZSortValue] )
	{
		// the current z-sort list is empty. (contains no copy entity)
		pCopyEnt->m_pNextEntityInZSortTable = NULL;
		m_apZSortTable[iZSortValue] = pCopyEnt;
		return;
	}
	else if( fZinCameraSpace < m_apZSortTable[iZSortValue]->fZinCameraSpace )	// compare z with the 1st copy entity in the current z-sort list
	{
		// 'pCopyEnt' has the minimum z value and should be placed at the head of the z-sort list
		pCopyEnt->m_pNextEntityInZSortTable = m_apZSortTable[iZSortValue];
		m_apZSortTable[iZSortValue] = pCopyEnt;
		return;
	}
	else
	{	// start searching from 2nd copy entity in this z-sort list
		pZSortedEntity = m_apZSortTable[iZSortValue]->m_pNextEntityInZSortTable;
		pPrevZSortedEntity = m_apZSortTable[iZSortValue];
		while(1)
		{
			if( !pZSortedEntity || fZinCameraSpace < pZSortedEntity->fZinCameraSpace )
			{
				pPrevZSortedEntity->m_pNextEntityInZSortTable = pCopyEnt;
				pCopyEnt->m_pNextEntityInZSortTable = pZSortedEntity;
				break;
			}
			pPrevZSortedEntity = pZSortedEntity;
			pZSortedEntity = pZSortedEntity->m_pNextEntityInZSortTable;
		}
		return;
	}

}


void EntityRenderManager::RenderScene( Camera& rCam )
{
	PROFILE_FUNCTION();

	//==================== render the entities ====================

	EntityNode::ms_NumRenderedEntities = 0;

	// move the skybox to the head of the list to render it first
	MoveSkyboxToListHead();

	if( true )//m_EnableFog )
	{
		UpdateFogParams( rCam );
		GraphicsDevice().Enable( RenderStateType::FOG );
	}

	// render the entity tree by downward traversal
	StandardEntityRenderer entity_renderer( this );
	if( m_paEntityTree )
		m_paEntityTree[0].RenderEntitiesWithDownwardTraversal( m_paEntityTree, entity_renderer, rCam, true );

	// render entities that have translucent polygons
	// NOTE: commenting out the this line alone will cause infinite loop because the z-sort table
	// is cleared in RenderZSortTable()
	RenderZSortTable();

	// Clear the Z-sort table.
	// Note that the Z-sort table must be cleared every frame whether or not RenderZSortTable() is called.
	ClearZSortTable();

	// render groups of copy entities that belong to a same base entity and should be rendered in succession 
	// e.g. bullet hold decals
	size_t i, num_sweeprender_base_entities = m_vecpSweepRenderBaseEntity.size();
	for(i=0; i<num_sweeprender_base_entities; i++)
	{
		m_vecpSweepRenderBaseEntity[i]->SweepRender();
		m_vecpSweepRenderBaseEntity[i]->ClearSweepRenderTable();
	}
}


void EntityRenderManager::RenderAllButEnvMapTarget( Camera& rCam, U32 target_entity_id )
{
	//==================== render the entities ====================

	EntityNode::ms_NumRenderedEntities = 0;

	// render the entity tree by downward traversal
	CNonEnvMapTargetEntityRenderer non_em_tgt_renderer( target_entity_id );
	if( m_paEntityTree )
		m_paEntityTree[0].RenderEntitiesWithDownwardTraversal( m_paEntityTree, non_em_tgt_renderer, rCam, true );

	// render entities that have translucent polygons
	RenderZSortTable();

	ClearZSortTable();

	// render groups of copy entities that belong to a same base entity and should be rendered in succession 
	// e.g. bullet hold decals
	size_t i, num_sweeprender_base_entities = m_vecpSweepRenderBaseEntity.size();
	for(i=0; i<num_sweeprender_base_entities; i++)
	{
		m_vecpSweepRenderBaseEntity[i]->SweepRender();
		m_vecpSweepRenderBaseEntity[i]->ClearSweepRenderTable();
	}
}


void EntityRenderManager::RenderShadowCasters( Camera& rCam )
{
	// Skybox does not cast shadows, so we don't call MoveSkyboxToListHead() this time.

	// render the entity tree by downward traversal
	ShadowCasterEntityRenderer shadow_caster_entity_renderer;
	if( m_paEntityTree )
		m_paEntityTree[0].RenderEntitiesWithDownwardTraversal( m_paEntityTree, shadow_caster_entity_renderer, rCam, false );

	// render groups of copy entities that belong to a same base entity and should be rendered in succession 
	// e.g. bullet hold decals
/*	size_t i, num_sweeprender_base_entities = m_vecpSweepRenderBaseEntity.size();
	for(i=0; i<num_sweeprender_base_entities; i++)
	{
		m_vecpSweepRenderBaseEntity[i]->SweepRenderShadowCasters();
//		m_vecpSweepRenderBaseEntity[i]->ClearSweepRenderTable();
	}
*/
}


void EntityRenderManager::RenderShadowReceivers( Camera& rCam )
{
	//==================== render the entities ====================

	// render the entity tree by downward traversal
	ShadowReceiverEntityRenderer shadow_receiver_entity_renderer;
	if( m_paEntityTree )
		m_paEntityTree[0].RenderEntitiesWithDownwardTraversal( m_paEntityTree, shadow_receiver_entity_renderer, rCam, true );

	// render groups of copy entities that belong to a same base entity and should be rendered in succession 
	// e.g. bullet hold decals
/*	size_t i, num_sweeprender_base_entities = m_vecpSweepRenderBaseEntity.size();
	for(i=0; i<num_sweeprender_base_entities; i++)
	{
		m_vecpSweepRenderBaseEntity[i]->SweepRenderShadowReceivers();
//		m_vecpSweepRenderBaseEntity[i]->ClearSweepRenderTable();
	}
*/
}


void EntityRenderManager::SetLightForShadow( const string& light_entity_name )
{
	CCopyEntity *pLightEntity = m_pEntitySet->GetEntityByName( light_entity_name.c_str() );
	if( pLightEntity )
	{
		m_vecLightForShadow.push_back( pLightEntity->Self() );
	}
}


void EntityRenderManager::RenderSceneToCubeMap( Camera& camera )
{
	RenderAllButEnvMapTarget( camera, m_CurrentEnvMapTargetEntityID );
}

/*
void EntityRenderManager::RenderSceneWithShadowMap( Camera& rCam, 
													 ScreenEffectManager *pScreenEffectMgr )
{
	pScreenEffectMgr->RaiseEffectFlag( ScreenEffect::ShadowMap );

	// render shadow map
	// the entities that cast shadows to other entities are rendered in this phase
	// shader manager of shadow map is set to CShader (singleton)
	m_pShadowManager->BeginSceneShadowMap();

	RenderShadowCasters( rCam );

	m_pShadowManager->EndSceneShadowMap();


	m_pShadowManager->BeginSceneDepthMap();

	m_pShadowManager->SetSceneCamera( rCam );
	RenderShadowReceivers( rCam );

	m_pShadowManager->EndSceneDepthMap();


	// render the scene with shadowmap
	// entities that receive shadows are rendered with shaders that support the shadowmap
	m_pShadowManager->BeginScene();

	pScreenEffectMgr->SetShaderManager();

	RenderScene( rCam );

	m_pShadowManager->EndScene();


	// shadow map texture and scene depth texture should be ready
	// at this point since they are supposed to be rendered as separate
	// render tasks in advance
	// - just draw the fullscreen rect to render the scene with shadow
	m_pShadowManager->RenderSceneWithShadow();
}*/


void EntityRenderManager::UpdateEnvironmentMapTargets()
{
	if( m_vecEnvMapTarget.size() == 0 )
		return;

	if( !m_bEnableEnvironmentMap )
		return;

	// remove entities from target list if it has been already destroyed
	vector<EnvMapTarget>::iterator itr = m_vecEnvMapTarget.begin();
	while( itr != m_vecEnvMapTarget.end() )
	{
		if( !IsValidEntity(itr->m_pEntity)
		 || itr->m_pEntity->GetID() != itr->m_EntityID )
		{
			// the entity has already been destroyed
			// - remove from the list
			itr = m_vecEnvMapTarget.erase( itr );
		}
		else
			itr++;
	}
}


void EntityRenderManager::UpdateEnvironmentMapTextures()
{
	if( m_vecEnvMapTarget.size() == 0 )
		return;

	if( !m_bEnableEnvironmentMap )
		return;

	// envmap is enabled and there is at least one entity that needs envmap texture

	if( !m_pCubeMapManager
	 || !m_pCubeMapManager->IsReady() )
	{
		return; // envmap is not available
	}

	size_t num_envmap_targets = m_vecEnvMapTarget.size();

	if( 1 < num_envmap_targets )
		num_envmap_targets = 1;

	for( size_t i=0; i<num_envmap_targets; i++ )
	{
		// update env map texture

		m_CurrentEnvMapTargetEntityID = m_vecEnvMapTarget[i].m_EntityID;

		// EntityRenderManager::RenderSceneToCubeMap() is called 6 times in this call
		// to render the scene to cube map texture
		m_pCubeMapManager->RenderToCubeMap();

		m_CurrentEnvMapTargetEntityID = 0;
	}
}


Result::Name EntityRenderManager::AddPlanarReflector( EntityHandle<>& entity, const SPlane& plane )
{
	static uint num_max_planar_reflection_groups = 2;

	shared_ptr<CCopyEntity> pEntity = entity.Get();
	if( !pEntity )
		return Result::INVALID_ARGS;

	for( int i=0; i<(int)s_PlanarReflectionGroups.size(); i++ )
	{
		PlanarReflectionGroup& group = s_PlanarReflectionGroups[i];
		if( AlmostSamePlanes( group.m_Plane, plane ) )
		{
			// Found a group with almost the same plane
			pEntity->s1 = (short)i;
			group.AddEntity( entity );
			return Result::SUCCESS;
		}
	}

	if( num_max_planar_reflection_groups <= (uint)s_PlanarReflectionGroups.size() )
		return Result::UNKNOWN_ERROR;

/*	if( !m_pMirroredScene )
	{
		m_pMirroredScene = TextureRenderTarget::Create();
		bool initialized = m_pMirroredScene->InitScreenSizeRenderTarget();
		if( !initialized )
			LOG_PRINT_WARNING( " Failed to create a render target for mirrored scene." );
	}*/

	// If the argument plane is valid, just use it as reflection plane.
	// If not, guess the reflection plane from the mesh of the entity
	// If the has no mesh, create a reflection plane from the entity's position and up direction.
	Plane reflection_plane( Vector3(0,1,0), 0 );
	if( IsValidPlane( plane ) )
	{
		reflection_plane = plane;
	}
	else
	{
		shared_ptr<BasicMesh> pMesh = pEntity->m_MeshHandle.GetMesh();
		if( pMesh )
		{
			const AABB3 entity_mesh_aabb = pMesh->GetAABB();
			if( entity_mesh_aabb.GetExtents().x < 0.001f )      reflection_plane = Plane( Vector3(1,0,0), entity_mesh_aabb.GetCenterPosition().x );
			else if( entity_mesh_aabb.GetExtents().y < 0.001f ) reflection_plane = Plane( Vector3(0,1,0), entity_mesh_aabb.GetCenterPosition().y );
			else if( entity_mesh_aabb.GetExtents().z < 0.001f ) reflection_plane = Plane( Vector3(0,0,1), entity_mesh_aabb.GetCenterPosition().z );
		}
		else
		{
			Vector3 plane_normal = pEntity->GetWorldPose().matOrient.GetColumn(1);
			reflection_plane = Plane( plane_normal, Vec3Dot( plane_normal, pEntity->GetWorldPosition() ) );
		}
	}

	if( !IsValidPlane( reflection_plane ) )
		reflection_plane = Plane( Vector3(0,1,0), 0 );

	pEntity->s1 = (short)s_PlanarReflectionGroups.size();
	s_PlanarReflectionGroups.push_back( PlanarReflectionGroup() );
	s_PlanarReflectionGroups.back().Init();
	s_PlanarReflectionGroups.back().m_Plane = reflection_plane;
	s_PlanarReflectionGroups.back().AddEntity( entity );

	return Result::SUCCESS;
}


Result::Name EntityRenderManager::RemovePlanarReflector( EntityHandle<>& entity, bool remove_planar_refelection_group )
{
	for( int i=0; i<(int)s_PlanarReflectionGroups.size(); i++ )
	{
		PlanarReflectionGroup& group = s_PlanarReflectionGroups[i];
		Result::Name res = group.RemoveEntity( entity );
		if( res == Result::SUCCESS )
		{
			if( remove_planar_refelection_group
			 && group.m_Entities.empty() )
			{
				s_PlanarReflectionGroups.erase( s_PlanarReflectionGroups.begin() + i );
			}

			return Result::SUCCESS;
		}
//		else
//			continue; // The entity was not found: try the next group
	}

	return Result::INVALID_ARGS;
}


void EntityRenderManager::UpdatePlanarReflectionTexture( Camera& rCam, PlanarReflectionGroup& group )
{
	if( !group.m_pReflectionRenderTarget )
//	if( !m_pMirroredScene )
		return;

	// matrix settings

	Matrix44 mirror = Matrix44Mirror( group.m_Plane );

	Matrix44 view = rCam.GetCameraMatrix();
	GetShaderManagerHub().PushViewAndProjectionMatrices( view * mirror, rCam.GetProjectionMatrix() );

	GraphicsDevice().SetClipPlane( 0, group.m_Plane );

	GraphicsDevice().UpdateViewProjectionTransformsForClipPlane( 0, view, rCam.GetProjectionMatrix() );

	// set planar reflection texture
	group.m_pReflectionRenderTarget->SetRenderTarget();
//	m_pMirroredScene->SetRenderTarget();

	// render the mirrored scene
//	RenderSceneWithShadows( rCam );
	RenderScene( rCam );

	// restore the original render target
	group.m_pReflectionRenderTarget->ResetRenderTarget();
//	m_pMirroredScene->ResetRenderTarget();

//	PERIODICAL( 500, group.m_pReflectionRenderTarget->GetRenderTargetTexture().SaveTextureToImageFile( ".debug/mirrored_scene.bmp" ) );

	GetShaderManagerHub().PopViewAndProjectionMatrices();

	// Render the entities that belong to this planar reflection group
	// This is a draft version.
	// TODO: Use the entity tree for better performance.
/*	const int num_entities = (int)group.m_Entities.size();
	for( int i=0; i<num_entities; i++ )
	{
		shared_ptr<CCopyEntity> pEntity = group.m_Entities[i].Get();
		if( !pEntity )
			continue;

		pEntity->Draw();
	}*/
}


void EntityRenderManager::UpdatePlanarReflectionTextures( Camera& rCam )
{
	// Render all the polygons in the mirrored scene.
	GraphicsDevice().SetCullingMode( CullingMode::CLOCKWISE );

	GraphicsDevice().EnableClipPlane( 0 );

	for( int i=0; i<(int)s_PlanarReflectionGroups.size(); i++ )
	{
		m_CurrentlyRenderedPlanarReflectionSceneID = i;

		UpdatePlanarReflectionTexture( rCam, s_PlanarReflectionGroups[i] );
	}

	GraphicsDevice().DisableClipPlane( 0 );

	GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );
}


TextureHandle EntityRenderManager::GetPlanarReflectionTexture( CCopyEntity& entity )
{
	for( int i=0; i<(int)s_PlanarReflectionGroups.size(); i++ )
	{
		PlanarReflectionGroup& group = s_PlanarReflectionGroups[i];
		if( !group.m_pReflectionRenderTarget )
			continue;

		for( int j=0; j<(int)group.m_Entities.size(); j++ )
		{
			shared_ptr<CCopyEntity> pEntity = group.m_Entities[j].Get();
			if( !pEntity )
				continue;

			if( entity.GetID() == pEntity->GetID() )
			{
				if( !group.m_TextureUpdated && m_pCurrentCamera )
				{
					UpdatePlanarReflectionTexture( *m_pCurrentCamera, group );
					group.m_TextureUpdated = true;
				}

				return group.m_pReflectionRenderTarget->GetRenderTargetTexture();
			}
		}
	}

	return TextureHandle();
}


int EntityRenderManager::GetCurrentlyRenderedPlanarReflectionSceneID() const
{
	return m_CurrentlyRenderedPlanarReflectionSceneID;
}


void EntityRenderManager::RenderPlanarReflectionSurfaces()
{
}


void EntityRenderManager::RenderMirroredScene()
{
/*	const Matrix44 view = m_pCurrentCamera ? m_pCurrentCamera->GetCameraMatrix()     : Matrix44Identity();
	const Matrix44 proj = m_pCurrentCamera ? m_pCurrentCamera->GetProjectionMatrix() : Matrix44Identity();
	int num_planer_reflection_groups = (int)s_PlanarReflectionGroups.size();
	for( int i=0; i<num_planer_reflection_groups; i++ )
	{
		PlanarReflectionGroup& group = s_PlanarReflectionGroups[i];

		Plane reflection_plane = group.m_Plane;
//		Plane reflection_plane( Vector3(0,1,0), 0 );
		Matrix44 mirror = Matrix44Mirror( reflection_plane );

		GetShaderManagerHub().PushViewAndProjectionMatrices( view * mirror, proj );

		m_pMirroredScene->SetRenderTarget();

		GraphicsDevice().SetCullingMode( CullingMode::CLOCKWISE );
//		RenderReflectionClipPlane( //reflection_plane );
		GraphicsDevice().EnableClipPlane( i );
//		SetClipPlane( reflection_plane );
//		SetClipPlaneViaD3DXFunctions();

//		RenderReflectionSourceMeshes( GetMirroredPosition( Plane(Vector3(0,1,0),0), g_Camera.GetPosition() ) );

//		LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();
//		HRESULT hr = pd3dDev->SetRenderState( D3DRS_CLIPPLANEENABLE, 0 );
		GraphicsDevice().DisableClipPlane( i );

		GetShaderManagerHub().PopViewAndProjectionMatrices();

		m_pMirroredScene->ResetRenderTarget();
	}*/
}


TextureHandle EntityRenderManager::GetEnvMapTexture( U32 entity_id )
{
	if( m_pCubeMapManager )
//		return m_pCubeMapManager->GetCubeTexture();
		return TextureHandle();
	else
		return TextureHandle();

//	return TextureHandle();
}


bool EntityRenderManager::AddEnvMapTarget( CCopyEntity *pEntity )
{
	if( !IsValidEntity(pEntity) )
		return false;

	if( m_vecEnvMapTarget.size() == NUM_MAX_ENVMAP_TARGETS )
	{
		LOG_PRINT_ERROR( "cannot add any more env map targets" );
		return false;
	}

	m_vecEnvMapTarget.push_back( EnvMapTarget() );
	EnvMapTarget& target = m_vecEnvMapTarget.back();
	target.m_pEntity  = pEntity;
	target.m_EntityID = pEntity->GetID();
	int cube_tex_index = 0;
	target.m_pCubeMapTextureLoader.reset( new CubeTextureParamsLoader( pEntity->Self().lock(), this, cube_tex_index ) );

	if( pEntity->m_pMeshRenderMethod )
		pEntity->m_pMeshRenderMethod->SetShaderParamsLoaderToAllMeshRenderMethods( target.m_pCubeMapTextureLoader );

	// init cube map manager if this is the first target
	if( !m_pCubeMapManager )
	{
		LOG_PRINT( " - creating a cube map manager" );
		m_pCubeMapManager = new CubeMapManager();
		m_pCubeMapManager->Init();
		m_pCubeMapManager->SetCubeMapSceneRenderer( this );
	}

	return true;
}


void EntityRenderManager::SaveEnvMapTextureToFile( const std::string& output_image_filename )
{
	if( m_pCubeMapManager )
		m_pCubeMapManager->SaveCubeTextureToFile( output_image_filename );
}


bool EntityRenderManager::RemoveEnvMapTarget( CCopyEntity *pEntity )
{
	const size_t num_envmap_targets = m_vecEnvMapTarget.size();
	for( size_t i=0; i<num_envmap_targets; i++ )
	{
		if( m_vecEnvMapTarget[i].m_EntityID == pEntity->GetID() )
		{
			if( pEntity->m_pMeshRenderMethod )
			{
				pEntity->m_pMeshRenderMethod->RemoveShaderParamsLoaderFromAllMeshRenderMethods(
					m_vecEnvMapTarget[i].m_pCubeMapTextureLoader
					);
			}

			m_vecEnvMapTarget.erase( m_vecEnvMapTarget.begin() + i );

			return true;
		}
	}

	return false;
}


bool EntityRenderManager::EnableSoftShadow( float softness, int shadowmap_size )
{
	SafeDelete( m_pShadowManager );

//	m_pShadowManager = new VarianceShadowMapManager();
	m_pShadowManager = new ShadowMapManager();

	m_pShadowManager->SetSceneRenderer( m_pShadowMapSceneRenderer );

	bool initialized = m_pShadowManager->Init();
	if( !initialized )
	{
		// shadow map is not available
		// graphics card does not support float point buffer
		// or there is not enough graphics memory, etc.
		SafeDelete( m_pShadowManager );
		return false;
	}

	return true;
}


bool EntityRenderManager::EnableShadowMap( int shadow_map_size )
{
	SafeDelete( m_pShadowManager );

	m_pShadowManager = new ShadowMapManager();

	m_pShadowManager->SetSceneRenderer( m_pShadowMapSceneRenderer );

//	m_pShadowManager->SetShadowMapSize( shadow_map_size );

	bool shadow_map_mgr_initialized = m_pShadowManager->Init();
	if( !shadow_map_mgr_initialized )
	{
		// shadow map is not available
		// graphics card does not support float point buffer
		// or there is not enough graphics memory, etc.
		SafeDelete( m_pShadowManager );
		return false;
	}

	return true;
}


void EntityRenderManager::DisableShadowMap()
{
	SafeDelete( m_pShadowManager );
}


/// Find lights near camera and update shadow map settings for the found lights.
void EntityRenderManager::UpdateLightsForShadow()
{
	Vector3 vCamCenter = Vector3(0,0,0);
	if( m_pCurrentCamera )
	{
		vCamCenter = m_pCurrentCamera->GetPosition();
	}

	// clear the entity buffer
	m_vecpEntityBuffer.resize( 0 );

	float r = 200;
	AABB3 aabb;
	aabb.vMin = vCamCenter - Vector3(r,r,r);
	aabb.vMax = vCamCenter + Vector3(r,r,r);
	OverlapTestAABB aabb_test( aabb, &m_vecpEntityBuffer );

	/// collect only the light entities
	aabb_test.TargetEntityTypeID = CCopyEntityTypeID::LIGHT_ENTITY;
	aabb_test.GroupIndex = -1;

	// get light entities that are near the camera
	// and should be considered as lights for shadow
	m_pEntitySet->GetOverlappingEntities( aabb_test );

//	vector<float> vecLightScore;
//	vecLightScore.reserve

	const size_t num_entities = m_vecpEntityBuffer.size();
	for( size_t i=0; i<num_entities; i++ )
	{
		if( m_vecpEntityBuffer[i]->GetEntityTypeID() != CCopyEntityTypeID::LIGHT_ENTITY )
			continue;

		LightEntity *pLightEntity = dynamic_cast<LightEntity *>(m_vecpEntityBuffer[i]);
		if( !pLightEntity )
			continue;

		Light *pLight = pLightEntity->GetLightObject();
		if( !pLight )
			continue;

		m_pShadowManager->UpdateLightForShadow( pLightEntity->GetID(), *pLight );

		// found a light entity
//		m_pShadowManager->AddShadowForLight(
	}
}


void EntityRenderManager::ReleaseGraphicsResources()	
{
	size_t i, num_base_entities = m_pEntitySet->m_vecpBaseEntity.size();
	for(i=0; i<num_base_entities; i++)
		m_pEntitySet->m_vecpBaseEntity[i]->ReleaseGraphicsResources();

//	m_pEntitySet->m_pLightEntityManager->ReleaseGraphicsResources();

	// m_pCubeMapManager - graphics component
	// m_pShadowManager - graphics component
}


void EntityRenderManager::LoadGraphicsResources( const GraphicsParameters& rParam )
{
	size_t i, num_base_entities = m_pEntitySet->m_vecpBaseEntity.size();
	for(i=0; i<num_base_entities; i++)
		m_pEntitySet->m_vecpBaseEntity[i]->LoadGraphicsResources( rParam );

//	m_pEntitySet->m_pLightEntityManager->LoadGraphicsResources();

	// m_pCubeMapManager - graphics component
	// m_pShadowManager - graphics component
}



/**
 - Render the shadow caster entities to shadow map texture(s).
 - Render the shadow receiver entities.
 - Render the scene without shadow to render target texture.
*/
void EntityRenderManager::RenderSceneWithShadows( Camera& rCam )//, 
//													 ScreenEffectManager *pScreenEffectMgr )
{
	if( m_bOverrideShadowMapLight )
	{
//		m_pShadowManager->SetLightPosition( m_vOverrideShadowMapPosition );
//		m_pShadowManager->SetLightDirection( m_vOverrideShadowMapDirection );
	}

	static float near_clip = 0.001f;
	static float far_clip  = 100.0f;
	UPDATE_PARAM( "debug/graphics_params.txt", "shadowmap_scene_cam_nearclip", near_clip );
	UPDATE_PARAM( "debug/graphics_params.txt", "shadowmap_scene_cam_farclip",  far_clip );
	m_pShadowManager->SetCameraPosition( rCam.GetPosition() );
	m_pShadowManager->SetCameraDirection( rCam.GetFrontDirection() );
	m_pShadowManager->SceneCamera().SetNearClip( near_clip );
	m_pShadowManager->SceneCamera().SetFarClip( far_clip );

	// render the objects that cast shadows to others

	// EntityRenderManager::RenderShadowCasters() is called 1 or more times
	m_pShadowManager->RenderShadowCasters( rCam );
/*
	// render shadow map
	// the entities that cast shadows to other entities are rendered in this phase
	// shader manager of shadow map is set to CShader (singleton)
	m_pShadowManager->BeginSceneShadowMap();

	// render to shadow map texture
	// BeginScene() and EndScene() pair is called inside
	RenderShadowCasters( rCam );

	m_pShadowManager->EndSceneShadowMap();
*/

	m_pShadowManager->SetSceneCamera( rCam );
	m_pShadowManager->SceneCamera().SetNearClip( near_clip );
	m_pShadowManager->SceneCamera().SetFarClip( far_clip );

	// render the objects that receive shadows

	m_pShadowManager->RenderShadowReceivers( rCam );
/*
	m_pShadowManager->BeginSceneDepthMap();

	// Render to scene depth texture
	// BeginScene() and EndScene() pair is called inside

	RenderShadowReceivers( rCam );

	m_pShadowManager->EndSceneDepthMap();
*/

	// set texture render target and call IDirect3DDevice9::BeginScene();
	m_pShadowManager->BeginScene();

	RenderScene( rCam );

	// call IDirect3DDevice9::EndScene() and reset the prev render target;
	m_pShadowManager->EndScene();

	// Render the scene as fullscreen rect with the scene texture and shadow overlay texture
	m_pShadowManager->RenderSceneWithShadow();
}


void EntityRenderManager::CreateEnvMapRenderTasks()
{
	RenderTaskProcessor.AddRenderTask( new CEntityEnvMapRenderTask( this ) );
}


void EntityRenderManager::CreateShadowMapRenderTasks( Camera& rCam )
{
	RenderTaskProcessor.AddRenderTask( new CEntityShadowMapRenderTask( this, &rCam, NULL ) );
}


void EntityRenderManager::CreateSceneRenderTask( Camera& rCam )
{
	RenderTaskProcessor.AddRenderTask( new CEntitySceneRenderTask( this, &rCam ) );
}


void EntityRenderManager::CreateRenderTasks( bool create_scene_render_task )
{
	if( 0 < m_vecEnvMapTarget.size() 
	 && m_bEnableEnvironmentMap
	 && m_pCubeMapManager
	 && m_pCubeMapManager->IsReady() )
	{
		UpdateEnvironmentMapTargets();

		CreateEnvMapRenderTasks();
	}

	if( this->m_pShadowManager )
	{
		// the scene needs to be rendered to texture render target in advance
		// - it will be later drawn as fullscreen rect blended with shadow overlay texture
		CreateShadowMapRenderTasks( *m_pCurrentCamera );
	}
	else
	{
		// scene is rendered directly to the original render target
		// by Render() call
	}

	if( create_scene_render_task )
		CreateSceneRenderTask( *m_pCurrentCamera );
}


void EntityRenderManager::UpdateFogParams( const Camera& rCam )
{
	FogParams fog_params;

	float far_clip_margin = 10.0f;
	fog_params.End = rCam.GetFarClip() - far_clip_margin;

	fog_params.Start = fog_params.End * 0.5f;

	if( m_paEntityTree
	 && m_paEntityTree[0].m_EntityLinkHead.pNext
	 && m_paEntityTree[0].m_EntityLinkHead.pNext->pOwner->pBaseEntity->GetArchiveObjectID() == BaseEntity::BE_SKYBOX )
	{
		// TODO: have the copy entity of skybox
		CBE_Skybox *pBaseEntity = dynamic_cast<CBE_Skybox *>(m_paEntityTree[0].m_EntityLinkHead.pNext->pOwner->pBaseEntity);
		if( pBaseEntity )
		{
			SFloatRGBAColor color( 0.7f, 0.7f, 0.7f, 1.0f );
			pBaseEntity->GetFogColor( color );
			fog_params.Color = color;
		}
	}

	GraphicsDevice().SetFogParams( fog_params );
}


void EntityRenderManager::Render( Camera& rCam )
{
	PROFILE_FUNCTION();

	m_pEntitySet->UpdateGraphics();

	// update camera matrix & camera's position
	rCam.GetPose( m_CameraPose );
	m_fCameraFarClipDist = rCam.GetFarClip();

//	Matrix44 matView, matProj;
//	rCam.GetCameraMatrix(matView);
//	rCam.GetProjectionMatrix(matProj);

//	FixedFunctionPipelineManager().SetWorldTransform( Matrix44Identity() );
//	FixedFunctionPipelineManager().SetViewTransform( matView );
//	FixedFunctionPipelineManager().SetProjectionTransform( matProj );

	/// set view and projection matrices to all shader managers
	GetShaderManagerHub().PushViewAndProjectionMatrices( rCam );

	// clear dest buffer
	GraphicsDevice().SetClearColor( SFloatRGBAColor(0.3f,0.3f,0.3f,1.0f) );
	GraphicsDevice().SetClearDepth( 1.0f );
	GraphicsDevice().Clear( BufferMask::COLOR | BufferMask::DEPTH );

	GraphicsDevice().SetRenderState( RenderStateType::DEPTH_TEST, true );

	// set alpha test
	GraphicsDevice().Enable( RenderStateType::ALPHA_TEST );
	GraphicsDevice().SetAlphaFunction( CompareFunc::GREATER_THAN_OR_EQUAL_TO );
	GraphicsDevice().SetReferenceAlphaValue( 1.0f / 256.0f );

	// enable lights of graphics device for entities
	GraphicsDevice().SetRenderState( RenderStateType::LIGHTING, true );

	GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );

	// test - add some ambient light
//	pd3dDev->SetRenderState( D3DRS_AMBIENT, 0x00202020 );

	if( m_pShadowManager )
	{
		// A new shadowmap may be created in this function. This must be called before m_pShadowManager->HasShadowMap().
		// The same light is used for the shadowing both in the real world and the mirrored world.
		UpdateLightsForShadow();
	}

	m_CurrentlyRenderedPlanarReflectionSceneID = -1;

	// create env map texture based no the current camera pose
	bool do_not_use_render_task = true;
	if( do_not_use_render_task )
	{
//		if( flags & ENVIRONMENT_MAPPING )
		if( m_bEnableEnvironmentMap )
		{
			UpdateEnvironmentMapTargets();
			UpdateEnvironmentMapTextures();
		}

		if( !s_PlanarReflectionGroups.empty() )
			UpdatePlanarReflectionTextures( rCam );
	}

	if( m_pShadowManager // A
	 && m_pShadowManager->HasShadowMap() ) // B
	{
		RenderSceneWithShadows( rCam );
	}
	else
	{
		// Directly render the scene to the currenet render target.
		// This happens if one of the following is true:
		// 1) Shadowmap is disabled (A is false).
		// 2) Shadowmap is enabled, but there is no light for the shadowmap (B is false).
		RenderScene( rCam );
	}

	GetShaderManagerHub().PopViewAndProjectionMatrices();

	static int s_SaveSceneTextureOfShadowMapToFile = 0;
	if( s_SaveSceneTextureOfShadowMapToFile )
	{
		if( m_pShadowManager )
			m_pShadowManager->SaveSceneTextureToFile( "debug/scene_for_shadowmap.bmp" );
		s_SaveSceneTextureOfShadowMapToFile = 0;
	}

	// >>> experiment - render planar reflectors
	for( int i=0; i<(int)s_PlanarReflectionGroups.size(); i++ )
	{
		PlanarReflectionGroup& group = s_PlanarReflectionGroups[i];
		for( int j=0; j<(int)group.m_Entities.size(); j++ )
		{
			shared_ptr<CCopyEntity> pEntity = group.m_Entities[j].Get();
			if( !pEntity )
				continue;

			pEntity->Draw();
		}
	}
	// <<< experiment - render planar reflectors
}


} // namespace amorphous
