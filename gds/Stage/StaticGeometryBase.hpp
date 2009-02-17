#ifndef __StaticGeometryBase_H__
#define __StaticGeometryBase_H__


#include "fwd.hpp"
#include "Graphics/fwd.hpp"
#include "Graphics/FloatRGBAColor.hpp"
#include "Graphics/GraphicsComponentCollector.hpp"
#include "3DMath/Vector3.hpp"
#include "3DMath/AABB3.hpp"
#include "Physics/fwd.hpp"

#include <vector>
#include <string>


class CStaticGeometryBase : public CGraphicsComponent
{

public:

//	enum eBaseStaticGeometryParam	{	};

protected:

	std::string m_strFilename;

	/// axis-aligned bounding box that contains the entire geometry
	AABB3 m_AABB;

	/// stage object that uses this static geometry
	CStage* m_pStage;

/*	enum eExtraTextureType
	{
		EXTRA_TEX_TRANSPARENT,
		EXTRA_TEX_TRANSLUCENT,
		EXTRA_TEX_SPECULAR,
		EXTRA_TEX_DEFAULT_COLOR,
		EXTRA_TEX_DEFAULT_NORMALMAP,
		NUM_EXTRA_TEXTURES
	};

	CTextureHandle m_apExtraTexture[NUM_EXTRA_TEXTURES];

	CDynamicLightManagerForStaticGeometry *m_pDynamicLightManager;
	CTextureHandle m_Lightmap_PL;	///< lightmap texture for point light
	CTextureHandle m_Normalmap_PL;	///< normal map texture for point light
*/

public:

	CStaticGeometryBase( CStage *pStage ) : m_pStage(pStage) { m_AABB.Nullify(); }

	virtual ~CStaticGeometryBase() {}

	virtual int GetType() const = 0;

	const AABB3& GetAABB() const { return m_AABB; }

	virtual bool Render( const CCamera& rCam, const unsigned int EffectFlag ) = 0;

	virtual bool LoadFromFile( const std::string& filename, bool bLoadGraphicsOnly = false ) = 0;

	virtual void WriteToFile( const char *pFilename ) {}

	virtual void MakeEntityTree( CBSPTree& bsptree );

	/// collision detection
	virtual int ClipTrace(STrace& tr) = 0;

	virtual short CheckPosition(STrace& tr) = 0;

	virtual char IsCurrentlyVisibleCell( short sCellIndex ) = 0;

	virtual void SetAmbientColor( const SFloatRGBAColor& ambient_color ) {}

	virtual void SetFogColor( const SFloatRGBAColor& color ) {}

	virtual void SetFogStartDist( float dist ) {}

	virtual void SetFogEndDist( float dist ) {}

	virtual void SetDynamicLightManager( CEntitySet* pEntitySet ) {}

	/// used for dynamic lighting
	virtual void GetIntersectingPolygons( Vector3& vCenter,
		                                  float fRadius,
										  AABB3& aabb,
										  std::vector<int>& veciLitPolygonIndex ) {}

	virtual physics::CActor *CreateCollisionGeometry( physics::CScene& physics_scene ) { return NULL; }

	virtual void ReleaseGraphicsResources() {}
	virtual void LoadGraphicsResources( const CGraphicsParameters& rParam ) {}


	enum type
	{
		TYPE_BSPMAP,
		TYPE_FLIGHTGAME,
		TYPE_GENERAL,
		NUM_TYPES
	};

	// The following three (five?) functions are used for dynamic light mapping

	// allows access to each map vertex
	// LockVertexBuffer() has to be called before calling this function?
/*	inline MAPVERTEX *GetVertex();

	inline bool LockVertexBuffer();

	inline void UnlockVertexBuffer();

	inline CSG_Polygon *GetPolygon() {	return m_paPolygon; }
	inline CSG_Polygon *GetPolygon(int iIndex) { return &m_paPolygon[iIndex]; }
*/

};


#endif  /*  __StaticGeometryBase_H__  */
