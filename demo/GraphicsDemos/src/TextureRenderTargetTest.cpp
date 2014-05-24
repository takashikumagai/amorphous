#include "TextureRenderTargetTest.hpp"
#include "amorphous/Graphics/GraphicsDevice.hpp"
#include "amorphous/Graphics/Shader/GenericShaderGenerator.hpp"
#include "amorphous/Graphics/PrimitiveShapeRenderer.hpp"
#include "amorphous/Graphics/HemisphericLight.hpp"
#include "amorphous/Graphics/TextureRenderTarget.hpp"
#include "amorphous/Graphics/Camera.hpp"
#include "amorphous/Graphics/MeshGenerators/MeshGenerators.hpp"
#include "amorphous/Graphics/MeshUtilities.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/FixedFunctionPipelineManager.hpp"
#include "amorphous/Graphics/Shader/ShaderLightManager.hpp"
#include "amorphous/Graphics/Shader/ShaderManagerHub.hpp"
#include "amorphous/Graphics/Shader/GenericShaderHelpers.hpp"
#include "amorphous/Support/Profile.hpp"
#include "amorphous/Support/ParamLoader.hpp"
#include "amorphous/Support/Macro.h"
#include "amorphous/Support/Timer.hpp"

using std::string;
using std::vector;
using boost::shared_ptr;
using namespace boost;


CTextureRenderTargetTest::CTextureRenderTargetTest()
{
	m_MeshTechnique.SetTechniqueName( "Default" );

	SetBackgroundColor( SFloatRGBAColor( 0.2f, 0.2f, 0.5f, 1.0f ) );

	if( CameraController() )
		CameraController()->SetPosition( Vector3( 0, 1, -3 ) );
}


CTextureRenderTargetTest::~CTextureRenderTargetTest()
{
}


bool CTextureRenderTargetTest::InitShader()
{
	// initialize shader
//	bool shader_loaded = m_Shader.Load( "./shaders/PerPixelSingleHSDirectionalLight.fx" );
//	bool shader_loaded = m_Shader.Load( "./shaders/null.fx" );

	ShaderResourceDesc shader_desc;
	GenericShaderDesc gen_shader_desc;
	gen_shader_desc.LightingTechnique = ShaderLightingTechnique::HEMISPHERIC;
	shader_desc.pShaderGenerator.reset( new GenericShaderGenerator(gen_shader_desc) );
	bool shader_loaded = m_Shader.Load( shader_desc );

	m_NoLightingShader = CreateNoLightingShader();
	
//	if( !shader_loaded )
//		return false;

	ShaderManager *pShaderMgr = m_Shader.GetShaderManager();

	ShaderManager& shader_mgr = pShaderMgr ? *pShaderMgr : FixedFunctionPipelineManager();

	shared_ptr<ShaderLightManager> pShaderLightMgr = shader_mgr.GetShaderLightManager();

	if( pShaderLightMgr )
	{
		HemisphericDirectionalLight light;
		light.Attribute.UpperDiffuseColor.SetRGBA( 1.0f, 1.0f, 1.0f, 1.0f );
		light.Attribute.LowerDiffuseColor.SetRGBA( 0.2f, 0.2f, 0.2f, 1.0f );
		light.vDirection = Vec3GetNormalized( Vector3( -1.0f, -1.5f, -0.9f ) );

		pShaderLightMgr->SetHemisphericDirectionalLight( light );
		pShaderLightMgr->CommitChanges();
	}

	return true;
}


int CTextureRenderTargetTest::Init()
{
	InitShader();

	TextureResourceDesc tex_desc;
	tex_desc.Width  = 1280;
	tex_desc.Height = 720;
	tex_desc.Format = TextureFormat::A8R8G8B8;
	tex_desc.UsageFlags = UsageFlag::RENDER_TARGET;
	tex_desc.MipLevels  = 1;

	m_pTextureRenderTarget = TextureRenderTarget::Create();
	bool res = m_pTextureRenderTarget->Init( tex_desc );
//	bool res = m_pTextureRenderTarget->Init( 1280, 720 );
	if( !res )
		LOG_PRINT_ERROR( "Failed to create a render target texture." );

	BoxMeshGenerator generator;
	generator.Generate( Vector3(1,1,1), MeshGenerator::DEFAULT_VERTEX_FLAGS, SFloatRGBAColor::White() );
	C3DMeshModelArchive& box_mesh_archive = generator.MeshArchive();
	m_BoxMesh.LoadFromArchive( box_mesh_archive );
	bool tex_loaded = false;
	if( 0 < m_BoxMesh.GetNumMaterials() )
	{
//		CMeshMaterial& mat = m_BoxMesh.Material(0);
//		mat.Texture.resize(1);
//		mat.TextureDesc.resize(1);
//		mat.TextureDesc[0].pLoader.reset( new CSingleColorTextureFilling() );
//		mat.Texture[0].Load( mat.TextureDesc[0] );
//		tex_loaded = m_BoxMesh.Material(0).Texture[0].Load( "textures/SmashedGrayMarble.jpg" );
	}
	Set6FaceColors( m_BoxMesh );

	m_Mesh.Load( "Common/models/wc1_1.00.msh" );

	return 0;
}


void CTextureRenderTargetTest::Update( float dt )
{
}


void CTextureRenderTargetTest::RenderMeshes()
{
	GraphicsDevice().SetRenderState( RenderStateType::DEPTH_TEST, true );
//	GraphicsDevice().Enable( RenderStateType::LIGHTING );

	ShaderManager *pShaderManager = m_Shader.GetShaderManager();

	ShaderManager& shader_mgr = pShaderManager ? *pShaderManager : FixedFunctionPipelineManager();

//	shared_ptr<ShaderLightManager> pLightMgr = shader_mgr.GetShaderLightManager();
//	if( pLightMgr )

	// render the scene

	shader_mgr.SetViewerPosition( GetCurrentCamera().GetPosition() );

	Camera cam;
	cam.SetAspectRatio( 16.0f / 9.0f );
	cam.SetPose( Matrix34( Vector3( 0, 1, -3 ), Matrix33Identity() ) );

	GetShaderManagerHub().PushViewAndProjectionMatrices( cam );

	Result::Name res = shader_mgr.SetTechnique( m_MeshTechnique );

	double current_time = GlobalTimer().GetTime();

	Matrix34 world_transform( Vector3(0,1,3), Matrix33RotationY((float)current_time) * Matrix33RotationX((float)current_time * 0.3f) );
	shader_mgr.SetWorldTransform( world_transform );
	m_BoxMesh.Render( shader_mgr );

	PrimitiveShapeRenderer renderer;
	renderer.SetShader( m_Shader );
	world_transform = Matrix34( Vector3(3,1,3), Matrix33RotationX((float)current_time) * Matrix33RotationY((float)current_time * 0.2f) );
	shader_mgr.SetWorldTransform( world_transform );
	renderer.RenderBox( Vector3(1,1,1), world_transform );

	shared_ptr<BasicMesh> pMesh = m_Mesh.GetMesh();
	if( pMesh )
	{
		world_transform = Matrix34( Vector3(-3,1,3), Matrix33RotationZ((float)current_time * 0.3f) * Matrix33RotationY((float)current_time * 0.6f) );
		shader_mgr.SetWorldTransform( world_transform );
		pMesh->Render( shader_mgr );
	}

	GetShaderManagerHub().PopViewAndProjectionMatrices();
}


void CTextureRenderTargetTest::RenderTexturedRect()
{
	if( !m_pTextureRenderTarget )
		return;

	TextureHandle render_target_texture = m_pTextureRenderTarget->GetRenderTargetTexture();
	Matrix34 pose( Matrix34Identity() );
	pose.vPosition = Vector3(0,1,3);
	pose.matOrient = Matrix33RotationY( (float)PI );
	float width  = 1.28f * 2.0f;
	float height = 0.72f * 2.0f;

	PrimitiveShapeRenderer renderer;
	renderer.SetShader( m_NoLightingShader );
	renderer.RenderPlane( pose, width, height, SFloatRGBAColor::White(), render_target_texture );
}


void CTextureRenderTargetTest::Render()
{
	PROFILE_FUNCTION();

	// Render a mesh to a render target texxture

	if( m_pTextureRenderTarget )
	{
		m_pTextureRenderTarget->SetBackgroundColor( SFloatRGBAColor( 0.5f, 0.6f, 0.9f, 1.0f ) );

		m_pTextureRenderTarget->SetRenderTarget();

		RenderMeshes();

		m_pTextureRenderTarget->ResetRenderTarget();
	}

	// The mesh was rendered to the render target texture of m_pTextureRenderTarget

	RenderTexturedRect();
}


void CTextureRenderTargetTest::SaveTexturesAsImageFiles()
{
	if( m_pTextureRenderTarget )
		m_pTextureRenderTarget->GetRenderTargetTexture().SaveTextureToImageFile( ".debug/render_target.png" );
}


void CTextureRenderTargetTest::HandleInput( const InputData& input )
{
	switch( input.iGICode )
	{
	case GIC_F11:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
			SaveTexturesAsImageFiles();
		}
		break;
	case GIC_SPACE:
	case GIC_ENTER:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
		}
		break;
	default:
		CGraphicsTestBase::HandleInput( input );
		break;
	}
}