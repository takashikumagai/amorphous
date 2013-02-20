#include "SimpleOverlayEffectsTest.hpp"
#include <boost/foreach.hpp>
#include "gds/3DMath/Matrix34.hpp"
#include "gds/Graphics.hpp"
#include "gds/Graphics/TextureGenerators/NoiseTextureGenerators.hpp"
#include "gds/Support/Timer.hpp"
#include "gds/Support/Profile.hpp"
#include "gds/Support/ParamLoader.hpp"
#include "gds/Support/Macro.h"
#include "gds/Support/CameraController_Win32.hpp"
#include "gds/Input.hpp"
#include "gds/GUI.hpp"

using std::string;
using namespace boost;


CSimpleOverlayEffectsTest::CSimpleOverlayEffectsTest()
:
m_EnableNoiseEffect(true),
m_EnableStripeEffect(true),
m_DisplayResourceInfo(false)
{
	m_MeshTechnique.SetTechniqueName( "NoLighting" );

	SetBackgroundColor( SFloatRGBAColor( 0.2f, 0.2f, 0.5f, 1.0f ) );

	if( GetCameraController() )
		GetCameraController()->SetPosition( Vector3( 0, 5, -30 ) );
}


CSimpleOverlayEffectsTest::~CSimpleOverlayEffectsTest()
{
}


void CSimpleOverlayEffectsTest::CreateSampleUI()
{
}


bool CSimpleOverlayEffectsTest::InitShader()
{
	// initialize shader
	bool shader_loaded = m_Shader.Load( "./shaders/SimpleOverlayEffectsTest.fx" );
	
	ShaderManager& shader_mgr
		= (shader_loaded && m_Shader.GetShaderManager()) ? *(m_Shader.GetShaderManager()) : FixedFunctionPipelineManager();

//	if( !shader_loaded )
//		return false;

	Matrix44 proj = Matrix44PerspectiveFoV_LH( (float)PI / 4, 640.0f / 480.0f, 0.1f, 500.0f );
	shader_mgr.SetProjectionTransform( proj );

	ShaderLightManager *pShaderLightMgr = shader_mgr.GetShaderLightManager().get();
	if( !pShaderLightMgr )
		return false;

	HemisphericDirectionalLight light;
	light.Attribute.UpperDiffuseColor.SetRGBA( 1.0f, 1.0f, 1.0f, 1.0f );
	light.Attribute.LowerDiffuseColor.SetRGBA( 0.1f, 0.1f, 0.1f, 1.0f );
	light.vDirection = Vec3GetNormalized( Vector3( -1.0f, -1.8f, -0.9f ) );

//	pShaderLightMgr->SetLight( 0, light );
//	pShaderLightMgr->SetDirectionalLightOffset( 0 );
//	pShaderLightMgr->SetNumDirectionalLights( 1 );
	pShaderLightMgr->SetHemisphericDirectionalLight( light );

	return true;
}


int CSimpleOverlayEffectsTest::Init()
{
	m_pFont = CreateDefaultBuiltinFont();
	m_pFont->SetFontSize( 6, 12 );

	m_PseudoNoiseEffect.Init( 0.5f, 2 );

	// stripe texture
	TextureResourceDesc desc;
	desc.Width  = 16;
	desc.Height = 16;
	desc.Format = TextureFormat::A8R8G8B8;
	shared_ptr<StripeTextureGenerator> pGenerator( new StripeTextureGenerator );
	pGenerator->m_StripeWidth = 1;
	int w = 0;
	LoadParamFromFile( "params.txt", "stripe_width", w );
	if( 0 < w )
		pGenerator->m_StripeWidth = w;
	// opacity adjust by vertex diffuse color of the fullscreen rects
//	float alpha = 0.5f;
//	LoadParamFromFile( "params.txt", "stripe_opacity", alpha );
//	pGenerator->m_Color0 = SFloatRGBAColor(0,0,0,alpha);
//	pGenerator->m_Color0 = SFloatRGBAColor(0,0,0,1);
	desc.pLoader = pGenerator;
	bool loaded = m_StripeTexture.Load( desc );
/*
	m_MeshTechnique.SetTechniqueName( "NoLighting" );
	m_DefaultTechnique.SetTechniqueName( "NoShader" );
*/

	string model = "models/fw43.msh";
	LoadParamFromFile( "params.txt", "model", model );
	m_Meshes.resize( 1 );
	m_Meshes[0].Load( model );

	InitShader();

	return 0;
}


void CSimpleOverlayEffectsTest::Update( float dt )
{
}


void CSimpleOverlayEffectsTest::RenderMeshes()
{
	GraphicsDevice().Enable( RenderStateType::DEPTH_TEST );

	ShaderManager *pShaderMgr = m_Shader.GetShaderManager();

	ShaderManager& shader_mgr = pShaderMgr ? *pShaderMgr : FixedFunctionPipelineManager();

	// render the scene

	shader_mgr.SetViewerPosition( g_Camera.GetPosition() );

	GetShaderManagerHub().PushViewAndProjectionMatrices( g_Camera );

	shader_mgr.SetTechnique( m_MeshTechnique );
	BOOST_FOREACH( MeshHandle& mesh, m_Meshes )
	{
		shader_mgr.SetWorldTransform( Matrix44Identity() );

		BasicMesh *pMesh = mesh.GetMesh().get();

		if( pMesh )
			pMesh->Render( shader_mgr );
	}

	GetShaderManagerHub().PopViewAndProjectionMatrices_NoRestore();
}


void CSimpleOverlayEffectsTest::DisplayResourceInfo()
{
	GraphicsResourceManager().GetStatus( GraphicsResourceType::Texture, m_TextBuffer );

	Vector2 vTopLeft(     GetWindowWidth() / 4,  16 );
	Vector2 vBottomRight( GetWindowWidth() - 16, GetWindowHeight() * 3 / 2 );
	C2DRect rect( vTopLeft, vBottomRight, 0x50000000 );
	rect.Draw();

	m_pFont->DrawText( m_TextBuffer, vTopLeft );

	Vector3 vCamPos = g_Camera.GetPosition();
	m_pFont->DrawText(
		fmt_string( "x: %f\ny: %f\nz: %f\n", vCamPos.x, vCamPos.y, vCamPos.z ),
		Vector2( 20, 300 ) );
}


void CSimpleOverlayEffectsTest::Render()
{
	PROFILE_FUNCTION();

	RenderMeshes();

	m_PseudoNoiseEffect.SetNoiseTexture();

	if( m_EnableNoiseEffect )
		m_PseudoNoiseEffect.RenderNoiseEffect();

	if( m_EnableStripeEffect )
	{
		CViewport vp;
		GraphicsDevice().GetViewport( vp );
		C2DRect rect;
		static SFloatRGBAColor color( SFloatRGBAColor::White() );
		UPDATE_PARAM( "params.txt", "stripe_opacity", color.alpha );
		rect.SetColor( color );
		rect.SetPositionLTRB( 0, 0, (int)vp.Width-1, (int)vp.Height-1 );
//		rect.SetTextureUV( TEXCOORD2(0,0), TEXCOORD2(1,1) );
		rect.SetTextureUV( TEXCOORD2(0,0), TEXCOORD2((float)vp.Width/(float)16,(float)vp.Height/(float)16) );
		rect.Draw( m_StripeTexture );
	}

	if( m_DisplayResourceInfo )
		DisplayResourceInfo();
}


void CSimpleOverlayEffectsTest::HandleInput( const InputData& input )
{
	switch( input.iGICode )
	{
	case 'N':
		if( input.iType == ITYPE_KEY_PRESSED )
			m_EnableNoiseEffect = !m_EnableNoiseEffect;
		break;

	case 'H':
		if( input.iType == ITYPE_KEY_PRESSED )
			m_EnableStripeEffect = !m_EnableStripeEffect;
		break;

//	case GIC_F12:
//		if( input.iType == ITYPE_KEY_PRESSED )
//			SaveTexturesAsImageFiles();
//		break;

	case GIC_SPACE:
	case GIC_ENTER:
		if( input.iType == ITYPE_KEY_PRESSED )
		{
		}
		break;
	default:
		break;
	}
}


void CSimpleOverlayEffectsTest::ReleaseGraphicsResources()
{
//	m_pSampleUI.reset();
}


void CSimpleOverlayEffectsTest::LoadGraphicsResources( const GraphicsParameters& rParam )
{
//	CreateSampleUI();
}
