#include "SimpleMotionBlurTest.hpp"
#include "amorphous/Graphics/SimpleMotionBlur.hpp"
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"
#include "amorphous/Graphics/Font/BuiltinFonts.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/MeshGenerators/MeshGenerators.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/FixedFunctionPipelineManager.hpp"
#include "amorphous/Graphics/Camera.hpp"
#include "amorphous/Graphics/SkyboxMisc.hpp"
#include "amorphous/Support/CameraController.hpp"

using namespace boost;


CSimpleMotionBlurTest::CSimpleMotionBlurTest()
{
}


CSimpleMotionBlurTest::~CSimpleMotionBlurTest()
{
}


int CSimpleMotionBlurTest::Init()
{
	m_pFont = CreateDefaultBuiltinFont();

//	m_SkyboxTechnique.SetTechniqueName( "SkyBox" );
	m_MeshTechnique.SetTechniqueName( "NoLighting" );
	m_DefaultTechnique.SetTechniqueName( "NullShader" );

	// initialize shader
	bool shader_loaded = m_Shader.Load( "./shaders/SimpleMotionBlurTest.fx" );

	// load skybox mesh
//	m_SkyboxMesh = CreateSkyboxMesh( "./textures/skygrad_slim_01.jpg" );
	m_SkyTexture = CreateClearDaySkyTexture();
	m_SkyTexture.SaveTextureToImageFile( "sky.png" );

	// load the terrain mesh
	MeshResourceDesc mesh_desc;
	mesh_desc.ResourcePath = "./models/terrain06.msh";
	mesh_desc.MeshType     = MeshType::BASIC;
	m_TerrainMesh.Load( mesh_desc );

	if( CameraController() )
		CameraController()->SetPosition( Vector3( 0, 3, 0 ) );

	return 0;
}


void CSimpleMotionBlurTest::Update( float dt )
{
}


void CSimpleMotionBlurTest::RenderScene()
{
	Matrix44 matWorld = Matrix44Identity();
	ShaderManager *pShaderMgr = m_Shader.GetShaderManager();

	GraphicsDevice().Enable( RenderStateType::FACE_CULLING );
	GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );

	ShaderManager& shader_mgr = pShaderMgr ? (*pShaderMgr) : FixedFunctionPipelineManager();

//	RenderAsSkybox( m_SkyboxMesh, GetCurrentCamera().GetPose() );
	RenderSkybox( m_SkyTexture, GetCurrentCamera().GetPose() );

	shader_mgr.SetWorldTransform( matWorld );

	shader_mgr.SetTechnique( m_MeshTechnique );

	shared_ptr<BasicMesh> pTerrainMesh = m_TerrainMesh.GetMesh();
	if( pTerrainMesh )
		pTerrainMesh->Render( shader_mgr );
}


void CSimpleMotionBlurTest::Render()
{
	if( !m_pSimpleMotionBlur )
	{
		m_pSimpleMotionBlur.reset( new SimpleMotionBlur );
		m_pSimpleMotionBlur->InitForScreenSize();
		m_pSimpleMotionBlur->SetBlurWeight( 0.1f );
	}

	m_pSimpleMotionBlur->Begin();

	RenderScene();

	m_pSimpleMotionBlur->End();

	m_pSimpleMotionBlur->Render();
}
