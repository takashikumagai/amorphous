#ifndef  __ShadowMapTest_HPP__
#define  __ShadowMapTest_HPP__


#include "gds/3DMath/Vector3.hpp"
#include "gds/Graphics/fwd.hpp"
#include "gds/Graphics/MeshObjectHandle.hpp"
#include "gds/Graphics/TextureHandle.hpp"
#include "gds/Graphics/ShaderHandle.hpp"
#include "gds/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "gds/Graphics/ShadowMapManager.hpp"
#include "gds/Input/fwd.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class CShadowMapTestSceneRenderer;


class CShadowMapTest : public CGraphicsTestBase
{
	MeshHandle m_SkyboxMesh;

	MeshHandle m_Mesh;

	MeshHandle m_FloorMesh;

	ShaderTechniqueHandle m_MeshTechnique;

//	ShaderTechniqueHandle m_SkyboxTechnique;

	ShaderTechniqueHandle m_DefaultTechnique;

	ShaderHandle m_Shader;

	boost::shared_ptr<FontBase> m_pFont;

	boost::shared_ptr<ShadowMapManager> m_pShadowMapManager;

	boost::shared_ptr<Light> m_pLight;

	boost::shared_ptr<CShadowMapTestSceneRenderer> m_pShadowMapSceneRenderer;

private:

	void RenderScene( ShaderManager& shader_mgr );

public:

	CShadowMapTest();

	~CShadowMapTest();

	const char *GetAppTitle() const { return "ShadowMapTest"; }

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

	void HandleInput( const InputData& input );

	void RenderShadowCasters( Camera& camera );

	void RenderShadowReceivers( Camera& camera );
};


class CShadowMapTestSceneRenderer : public ShadowMapSceneRenderer
{
	CShadowMapTest *m_pTarget;

public:

	CShadowMapTestSceneRenderer( CShadowMapTest *pTarget ) : m_pTarget(pTarget) {}

	void RenderSceneToShadowMap( Camera& camera ) { m_pTarget->RenderShadowCasters( camera ); }

	void RenderShadowReceivers( Camera& camera ) { m_pTarget->RenderShadowReceivers( camera ); }
};



#endif /* __ShadowMapTest_HPP__ */
