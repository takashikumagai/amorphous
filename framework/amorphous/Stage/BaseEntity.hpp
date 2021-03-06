#ifndef __BaseEntity_HPP__
#define __BaseEntity_HPP__


#include "fwd.hpp"
#include "EntityGroupHandle.hpp"
#include "SharedMeshContainer.hpp"
#include "amorphous/Physics/fwd.hpp"
#include "amorphous/3DMath/AABB3.hpp"
#include "amorphous/3DMath/Matrix34.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/MeshObjectContainer.hpp"
#include "amorphous/Graphics/MeshContainerRenderMethod.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Graphics/Shader/Serialization_ShaderTechniqueHandle.hpp"
#include "amorphous/Support/TextFileScanner.hpp"
#include "amorphous/Support/Serialization/Serialization.hpp"


namespace amorphous
{

using namespace serialization;

class CoreBaseEntitiesLoader;


/**
 * base class of base entity
 */
class BaseEntity : public IArchiveObjectBase
{
protected:

	std::string m_strName;

	/// stage that owns this base entity
	CStageWeakPtr m_pStageWeakPtr;

	/// raw pointer for the stage for quick access
	CStage *m_pStage;

	SharedMeshContainer m_MeshProperty;

	/// used as a temporary array to hold shader techniques for mesh materials
	/// - See BaseEntity::DrawMeshObject()
	std::vector<ShaderTechniqueHandle> m_vecShaderTechniqueHolder;

	/// ENTITY_GROUP_MIN is set by default
	EntityGroupHandle m_EntityGroup;

	/// when copy entities should be rendered in succession, turn 'm_bSweepRender' to true
	/// and implement SweepRender() function (e.g. bullet hole decals)
	bool m_bSweepRender;
	std::vector<CCopyEntity *> m_vecpSweepRenderTable;

	/// flag that defines various attributes of a base entity
	unsigned int m_EntityFlag;

	/// radius of the bounding sphere
	float m_fRadius;

	/// axis aligned bounding box for the base entity (local coord)
	AABB3 m_aabb;

	/// type of the bounding volume
	char m_BoundingVolumeType;

	BSPTree *m_pBSPTree;	// for collision check against line segment

	/// true for non-collidable entities (ex. items)
	bool m_bNoClip;
	bool m_bNoClipAgainstMap;

	float m_fLife;

protected:

	//
	// Rendering
	//

	void DrawMeshObject( const Matrix34& world_pose,
						 BasicMesh *pMeshObject,
						 const std::vector<int>& vecTargetMaterialIndex,
						 array2d<ShaderTechniqueHandle>& rShaderTechHandleTable,
						 int ShaderLOD = 0 );

//	void DrawSkeletalMesh( CCopyEntity* pCopyEnt,
//		                   SkeletalMesh *pSkeletalMesh,
//		                   array2d<ShaderTechniqueHandle>& rShaderTechHandleTable,
//						   int ShaderLOD = 0 );

	/// \retval 0 shader for highest resolution mesh
	/// \retval higher_values shader for lower resolution model
	virtual int CalcShaderLOD( CCopyEntity* pCopyEnt ) { return 0; }

	void UpdateLightInfo( CCopyEntity& entity );

	void SetMeshRenderMethod( CCopyEntity& entity );

	void RenderEntity( CCopyEntity& entity );

public:

	BaseEntity();

	virtual ~BaseEntity();

	const char* GetName() const { return m_strName.c_str(); }

	const std::string& GetNameString() const { return m_strName; }

	void SetStagePtr( CStageWeakPtr pStage );


	//
	// Rendering
	//

	void Init3DModel();

	void CreateMeshGenerator( CTextFileScanner& scanner );

	// made public since alpha entity needs to call this
//	void DrawMeshSubset( const Matrix34& world_pose, int material_index, int ShaderLOD );

	// made public since alpha entity needs to call this
//	void DrawMeshSubset( const Matrix34& world_pose, int material_index, ShaderTechniqueHandle& shader_tech );

	/// draws a mesh object.
	/// For entities that have a single mesh object as their 3D model
	void Draw3DModel( CCopyEntity* pCopyEnt );

	void SetAsEnvMapTarget( CCopyEntity& entity );

	SharedMeshContainer& MeshProperty() { return m_MeshProperty; }

	/// loads a base entity on the memory from the disk
	/// base entities that use other base entities during runtime should
	/// pre-load them by calling this function
	bool LoadBaseEntity( BaseEntityHandle& base_entity_handle );

	inline void RaiseEntityFlag( const unsigned int flag ) { m_EntityFlag |= flag; }
	inline unsigned int GetEntityFlag() const { return m_EntityFlag; }
	inline void ClearEntityFlag( const unsigned int flag ) { m_EntityFlag &= (~flag); }

	float GetRadius() const { return m_fRadius; }

	/// Enable / disable the lighting to the entities created from this base entity
	void SetLighting( bool lighting );

	/// returns an id for an arbitrary entity group
    int GetEntityGroupID( EntityGroupHandle& entity_group_handle );

	/// returns entity group id of m_EntityGroup;
	/// - retrieves id from entity set if entity group handle is not initialized
	int GetEntityGroupID();

	void CategorizePosition(CCopyEntity* pCopyEnt);

	void NudgePosition(CCopyEntity* pCopyEnt);

	void FreeFall(CCopyEntity* pCopyEnt);

	char SlideMove(CCopyEntity* pCopyEnt);

	void GroundMove(CCopyEntity* pCopyEnt);

	void ApplyFriction(CCopyEntity* pCopyEnt, float fFriction);

	void ApplyFriction(float& rfSpeed, float fFriction);

	void Accelerate(CCopyEntity* pCopyEnt, Vector3& vWishdir, float& fWishspeed, float fAccel);

	void AirAccelerate(CCopyEntity* pCopyEnt, Vector3& vWishdir, float& fWishspeed, float fAccel);

	inline  void ClearSweepRenderTable() { m_vecpSweepRenderTable.resize(0); }

	virtual void Init() {}

	/// defines necessary initializations when a copy entity is created
	/// called after other properties (position/velocity, parent/child links, the entity flag, etc.)
	/// are initialized.
	virtual void InitCopyEntity( CCopyEntity* pCopyEnt ) {}

	/// difines the behavior of the entity during a frame
	virtual void Act(CCopyEntity* pCopyEnt) {}

	/// renders the copy entity
	virtual void Draw(CCopyEntity* pCopyEnt) {}

	virtual void RenderAsShadowCaster(CCopyEntity* pCopyEnt);

	virtual void RenderAsShadowReceiver(CCopyEntity* pCopyEnt);

	virtual void RenderAs( CCopyEntity& entity, CRenderContext& render_context );

	virtual void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other) {}

	virtual void ClipTrace( STrace& rTrace, CCopyEntity* pMyself );

	/// handles the message sent to the entity
	virtual void MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self) {}

	virtual void OnEntityDestroyed(CCopyEntity* pCopyEnt) {}

	/// called when the entity is set as camera entity for the stage
	virtual void RenderStage(CCopyEntity* pCopyEnt) {}

	virtual void CreateRenderTasks(CCopyEntity* pCopyEnt) {}

	/// load parameters from text file
	void LoadFromFile( CTextFileScanner& scanner );

	virtual bool LoadSpecificPropertiesFromFile( CTextFileScanner& scanner ) { return false; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	/// called when a copy entity of this base entity is being used as a camera entity
	virtual void UpdateCamera( CCopyEntity *pCopyEnt ) {}
	virtual Camera *GetCamera() { return NULL; }

	// Updates for properties that need to be calculated with fixed size timestep.
	// Mainly used for physics properties.
	virtual void UpdatePhysics( CCopyEntity *pCopyEnt, float dt ) {}

	virtual void UpdateScriptedMotionPath( CCopyEntity* pCopyEnt, CBEC_MotionPath& path );

	virtual void SweepRender() {}

//	void DrawMesh( CCopyEntity *pCopyEnt, int shader_tech_id = SHADER_TECH_INVALID );

	void CreateAlphaEntities( CCopyEntity *pCopyEnt );

	void InitEntityGraphics( CCopyEntity &entity,
                             ShaderHandle& shader = ShaderHandle(),
                             ShaderTechniqueHandle& tech = ShaderTechniqueHandle() );

	virtual void UpdateBaseEntity( float frametime )
	{
		int pass = 1;
	}

	/// implemented by physics base entity
	virtual const physics::CActorDesc& GetPhysicsActorDesc();

	virtual void AdaptToNewScreenSize() {}
	virtual void LoadGraphicsResources( const GraphicsParameters& rParam );
	virtual void ReleaseGraphicsResources();

	//
	// friend class
	//
	friend class EntityManager;
	friend class CoreBaseEntitiesLoader;

	enum BaseEntityID
	{
		BE_AREASENSOR,
		BE_BLAST,
		BE_BULLET,
		BE_CLOUD,
		BE_DECAL,
		BE_DOOR,
		BE_DOORCONTROLLER,
		BE_ENEMY,
		BE_EVENTTRIGGER,
		BE_EXPLOSIONSMOKE,
		BE_EXPLOSIVE,
		BE_FIXEDTURRETPOD,
		BE_FLOATER,
		BE_GENERALENTITY,
		BE_HOMINGMISSILE,
		BE_LASERDOT,
		BE_MUZZLEFLASH,
		BE_PARTICLESET,
		BE_PHYSICSBASEENTITY,
		BE_PLATFORM,
		BE_PLAYER,
		BE_PLAYERPSEUDOAIRCRAFT,
		BE_PLAYERPSEUDOLEGGEDVEHICLE,
		BE_PLAYERSHIP,
		BE_POINTLIGHT,
		BE_SMOKETRACE,
		BE_SUPPLYITEM,
		BE_TEXTUREANIMATION,
		BE_TURRET,
		BE_ENEMYAIRCRAFT,
		BE_ENEMYSHIP,
		BE_DIRECTIONALLIGHT,
		BE_SCRIPTEDCAMERA,
		BE_STATICPARTICLESET,
		BE_NOZZLEEXHAUST,       // 00:19 2007/04/18
		BE_STATICGEOMETRY,      // 17:27 2007/08/17
		BE_SKYBOX,              // 17:27 2007/08/17
		BE_CAMERACONTROLLER,    // 02:48 2007/09/09
		BE_INDIVIDUALENTITY,    // 01:23 2008/02/25
		BE_STATICLIQUID,        // 22:21 2011/04/19
		NUM_BASE_ENTITIES
	};

	enum eParams
	{
		USER_BASE_ENTITY_ID_OFFSET = NUM_BASE_ENTITIES
	};

};

} // namespace amorphous



#endif  /*  __BaseEntity_HPP__  */
