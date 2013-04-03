#ifndef  __LensFlareTest_HPP__
#define  __LensFlareTest_HPP__


#include "amorphous/3DMath/Vector3.hpp"
#include "amorphous/Graphics/fwd.hpp"
#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Input/fwd.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class CLensFlareTest : public CGraphicsTestBase
{
	boost::shared_ptr<LensFlare> m_pLensFlare;

	MeshHandle m_SkyboxMesh;

	MeshHandle m_TerrainMesh;

//	std::vector<MeshHandle> m_vecpMeshes;

	ShaderTechniqueHandle m_MeshTechnique;

//	ShaderTechniqueHandle m_SkyboxTechnique;

	ShaderTechniqueHandle m_DefaultTechnique;

	ShaderHandle m_Shader;

	TextureHandle m_TestTexture;

	TextureHandle m_SkyTexture;

	MeshHandle m_LightPosIndicator;

//	float m_FOV;// = PI / 4.0f;

	Vector3 m_vLightPosition;

	boost::shared_ptr<FontBase> m_pFont;

protected:

	void RenderFloor();

	void InitSkyTexture();
	
	void InitLensFlare( const std::string& strPath );

public:

	CLensFlareTest();

	~CLensFlareTest();

	const char *GetAppTitle() const { return "LensFlareTest"; }

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

	void UpdateViewTransform( const Matrix44& matView );

	void UpdateProjectionTransform( const Matrix44& matProj );

//	void OnKeyPressed( KeyCode::Code key_code );
};


#endif /* __LensFlareTest_HPP__ */
