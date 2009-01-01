#ifndef __ENTITYNODE_H__
#define __ENTITYNODE_H__


#include "../base.h"

#include "fwd.h"
#include "Graphics/fwd.h"
#include "CopyEntity.h"
#include "OverlapTestAABB.h"


/**
 * node for entity tree
 * - the entities are linked to the entity node
 * - used for both rendering and collision detection
 */
class CEntityNode
{
	bool leaf;
	SPlane m_Plane;

//	short m_sNumEnts;	// how many entities are currently in this sub-space
	short sFrontChild;
	short sBackChild;
	short sParent;

	/// link list of entities
	/// - The first entity of in the list is stored in m_EntityLinkHead.pNext
	/// - m_EntityLinkHead.pOwner and m_EntityLinkHead.pPrev is always NULL
	CLinkNode<CCopyEntity> m_EntityLinkHead;

	CLinkNode<CLightEntity> m_LightEntityLinkHead;

	short m_sCellIndex;	// index to the corresponding cell - only valid for complete leaf entity node

	/// bounding-box that represents sub-space of the node
	AABB3 m_AABB;

	CEntitySet* m_pEntitySet;

	CStage *m_pStage;

//	inline bool CheckCollisionGroup( int group_index, const std::vector<int>& vecTargetGroup );
	inline bool CheckCollisionGroup( int group0, int group1 );

public:

	static int ms_NumRenderedEntities;

	CEntityNode();

	void CheckPosition_r(STrace& tr, CEntityNode* paEntTree);
	void CheckPosition_r(CTrace& tr, CEntityNode* paEntTree);
	void GetVisibleEntities_r(CViewFrustumTest& vf_test, CEntityNode* paEntTree);
	void GetOverlappingEntities( COverlapTestAABB& overlap_test, CEntityNode* paEntTree );
	void ClipTrace_r(STrace& tr, CEntityNode* paEntTree);

	/// Link 'pEntity' to this entity node.
	/// 'pEntity' is set to the head element of the entity list of this entity node
	void Link(CCopyEntity* pEntity) { m_EntityLinkHead.InsertNext( &pEntity->m_EntityLink ); }

	/// render all the entites in this sub-space(entity tree node)
	void Render( CCamera& rCam );

	void RenderShadowCasters( CCamera& rCam );

	void RenderShadowReceivers( CCamera& rCam );

	void RenderAllButEnvMapTraget( CCamera& rCam, U32 target_entity_id  );


	/// link a light entity entity management
	void LinkLightEntity_r(CLightEntity *pLightEntity, CEntityNode* paEntTree);

	/// checks if the light is reaching the entity
	/// if it is true, registers the light index to the entity
	void CheckLight_r( CCopyEntity *pEntity, CEntityNode* paEntTree );

	friend class CEntityRenderManager;
	friend class CEntitySet;
};


#endif  /*  __ENTITYNODE_H__  */
