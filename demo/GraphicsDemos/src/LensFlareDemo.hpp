#ifndef  __LensFlareDemo_HPP__
#define  __LensFlareDemo_HPP__


#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Input/fwd.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class LensFlareDemo : public CGraphicsTestBase
{
	std::shared_ptr<LensFlare> m_pLensFlare;

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

protected:

	void RenderFloor();

	void InitSkyTexture();
	
	void InitLensFlare( const std::string& strPath );

public:

	LensFlareDemo();

	~LensFlareDemo();

	const char *GetAppTitle() const { return "LensFlareDemo"; }

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

	void UpdateViewTransform( const Matrix44& matView );

	void UpdateProjectionTransform( const Matrix44& matProj );

//	void OnKeyPressed( KeyCode::Code key_code );
};


#endif /* __LensFlareDemo_HPP__ */
