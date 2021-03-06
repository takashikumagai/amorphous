#include "SkyboxMisc.hpp"
#include "amorphous/Graphics/GraphicsDevice.hpp"
#include "amorphous/Graphics/MeshGenerators/MeshGenerators.hpp"
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"
#include "amorphous/Graphics/Mesh/CustomMesh.hpp"
#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/GenericShaderGenerator.hpp"
#include "amorphous/Graphics/TextureGenerators/GradationTextureGenerators.hpp"


namespace amorphous
{

using namespace std;


std::shared_ptr<BoxMeshGenerator> CreateSkyboxMeshGenerator( const std::string& texture_filepath )
{
	std::shared_ptr<BoxMeshGenerator> pBoxMeshGenerator( new BoxMeshGenerator );

	pBoxMeshGenerator->SetEdgeLengths( Vector3(1,1,1) );
	pBoxMeshGenerator->SetTexCoordStyleFlags( TexCoordStyle::LINEAR_SHIFT_INV_Y );
	pBoxMeshGenerator->SetPolygonDirection( MeshPolygonDirection::INWARD );
	pBoxMeshGenerator->SetTexturePath( texture_filepath );

	return pBoxMeshGenerator;
}


static std::shared_ptr<CylinderMeshGenerator> CreateSkyCylinderMeshGenerator( const std::string& texture_filepath )
{
	CylinderDesc desc;
	desc.style_flags |= CCylinderMeshStyleFlags::WELD_VERTICES;
	desc.num_sides = 32;
	desc.radii[0] = desc.radii[1] = 10.0f;
	desc.height = 10.0f;

	std::shared_ptr<CylinderMeshGenerator> pCylinderMeshGenerator( new CylinderMeshGenerator(desc) );

//	pCylinderMeshGenerator->SetTexCoordStyleFlags( TexCoordStyle::LINEAR_SHIFT_INV_Y );
	pCylinderMeshGenerator->SetTexCoordStyleFlags( TexCoordStyle::LINEAR_SHIFT_Y | TexCoordStyle::LINEAR_SHIFT_INV_Y );
	pCylinderMeshGenerator->SetPolygonDirection( MeshPolygonDirection::INWARD );
	pCylinderMeshGenerator->SetTexturePath( texture_filepath );

	return pCylinderMeshGenerator;
}


MeshHandle CreateSkyboxMesh( const std::string& texture_filepath )
{
	MeshResourceDesc skybox_mesh_desc;
	skybox_mesh_desc.pMeshGenerator = CreateSkyboxMeshGenerator( texture_filepath );

	MeshHandle skybox_mesh;
	bool loaded = skybox_mesh.Load( skybox_mesh_desc );

	return skybox_mesh;
}


template<class MeshClass>
void RenderAsSkybox( MeshClass& mesh, const Matrix34& vCamPose )
{
	Result::Name res;

	static ShaderHandle s_Shader;
	if( !s_Shader.IsLoaded() )
	{
		ShaderResourceDesc desc;
		GenericShaderDesc gs_desc;
		gs_desc.Lighting = false;
		std::shared_ptr<GenericShaderGenerator> pGenerator( new GenericShaderGenerator(gs_desc) );
		desc.pShaderGenerator = pGenerator;
		bool shader_loaded = s_Shader.Load( desc );
		if( !shader_loaded )
			LOG_PRINT_ERROR( " Failed to load the shader for rendering skyboxes." );
	}


	res = GraphicsDevice().Disable( RenderStateType::ALPHA_BLEND );
	res = GraphicsDevice().Disable( RenderStateType::ALPHA_TEST );
	res = GraphicsDevice().Disable( RenderStateType::LIGHTING );
	res = GraphicsDevice().Disable( RenderStateType::DEPTH_TEST );
	res = GraphicsDevice().Disable( RenderStateType::WRITING_INTO_DEPTH_BUFFER );
	res = GraphicsDevice().Enable(  RenderStateType::FACE_CULLING );
//	res = GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );

	// Commented out: don't change the "CullingMode::CLOCKWISE" mode when rendering the scene for a planar reflection texture.
//	res = GraphicsDevice().SetCullingMode( CullingMode::COUNTERCLOCKWISE );

//	ShaderManager& ffp_mgr = FixedFunctionPipelineManager();
	ShaderManager *pShaderMgr = s_Shader.GetShaderManager();
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = *pShaderMgr;

	const Vector3 vCamPos = vCamPose.vPosition;

	Matrix44 matWorld = Matrix44Identity();
	matWorld(0,3) = vCamPos.x;
	matWorld(1,3) = vCamPos.y;
	matWorld(2,3) = vCamPos.z;

	const Vector3 vCamUpDir  = vCamPose.matOrient.GetColumn(1);
	if( true )//0.000001 < fabs(vCamUpDir.y) )
	{
		const Vector3 vCamFwdDir = vCamPose.matOrient.GetColumn(2);
		const Vector3 vHDir = Vec3GetNormalized(Vector3(vCamFwdDir.x,0,vCamFwdDir.z));

		float heading = Vec3GetAngleBetween( Vector3(0,0,1), vHDir );
		if( Vec3Cross( Vector3(0,0,1), vHDir ).y < 0 )
			heading *= -1.0f;
		Matrix34 skybox_orientation( Vector3(0,0,0), Matrix33RotationY(heading) );

		// Apply the scaling to hide the both corners of the box.
		matWorld = matWorld * ToMatrix44(skybox_orientation) * Matrix44Scaling( 2.0f, 1.0f, 1.0f );
	}

	shader_mgr.SetWorldTransform( matWorld );

	mesh.Render( shader_mgr );

	res = GraphicsDevice().Enable( RenderStateType::DEPTH_TEST );
	res = GraphicsDevice().Enable( RenderStateType::WRITING_INTO_DEPTH_BUFFER );
}


void RenderAsSkybox( MeshHandle& mesh, const Matrix34& vCamPose )
{
	std::shared_ptr<BasicMesh> pMesh = mesh.GetMesh();

	if( pMesh )
		RenderAsSkybox( *pMesh, vCamPose );
}


void RenderSkyMesh( CustomMesh& mesh, TextureHandle& sky_texture, const Matrix34& vCamPose )
{
	if( 0 < mesh.GetNumMaterials() )
	{
		if( mesh.Material(0).Texture.empty() )
			mesh.Material(0).Texture.resize(1);

		mesh.Material(0).Texture[0] = sky_texture;
	}

	RenderAsSkybox( mesh, vCamPose );
}


void RenderSkybox( TextureHandle& sky_texture, const Matrix34& vCamPose )
{
	static CustomMesh s_SkyboxMesh;

	if( !s_SkyboxMesh.IsValid() )
	{
		std::shared_ptr<BoxMeshGenerator> pSkyboxMeshGenerator
			= CreateSkyboxMeshGenerator( "" );
		if( pSkyboxMeshGenerator )
		{
			pSkyboxMeshGenerator->Generate();
			C3DMeshModelArchive mesh_archive = pSkyboxMeshGenerator->GetMeshArchive();
			s_SkyboxMesh.LoadFromArchive( mesh_archive, "static_skybox_mesh", 0 );
		}
	}

	RenderSkyMesh( s_SkyboxMesh, sky_texture, vCamPose );
}


/// This does not work because the rim is obvious when the user looks up the sky.
/// Always use RenderSkybox()
/// I have a real heartbun for leaving this function knowing that it does not work
/// at least for rendering the sky, but decided to keep it for now as a reminder.
void RenderSkyCylinder( TextureHandle& sky_texture, const Matrix34& vCamPose )
{
	static CustomMesh s_SkyCylinderMesh;

	if( !s_SkyCylinderMesh.IsValid() )
	{
		std::shared_ptr<CylinderMeshGenerator> pSkyCylinderMeshGenerator
			= CreateSkyCylinderMeshGenerator( "" );
		if( pSkyCylinderMeshGenerator )
		{
			pSkyCylinderMeshGenerator->Generate();
			C3DMeshModelArchive mesh_archive = pSkyCylinderMeshGenerator->GetMeshArchive();
			s_SkyCylinderMesh.LoadFromArchive( mesh_archive, "static_sky_cylinder_mesh", 0 );
		}
	}

	RenderSkyMesh( s_SkyCylinderMesh, sky_texture, vCamPose );
}


TextureHandle CreateClearDaySkyTexture()
{
	return CreateHorizontalGradationTexture(
		256,
		256,
		TextureFormat::A8R8G8B8,
		SFloatRGBAColor( 0.573f, 0.651f, 0.678f, 1.0f ),
		SFloatRGBAColor( 0.950f, 0.980f, 0.999f, 1.0f ),
		SFloatRGBAColor( 0.620f, 0.545f, 0.522f, 1.0f )
		);
}


TextureHandle CreateCloudyDaySkyTexture()
{
	LOG_PRINT_ERROR( " Not implemented yet." );
	return TextureHandle();
}


TextureHandle CreateClearNightSkyTexture()
{
	LOG_PRINT_ERROR( " Not implemented yet." );
	return TextureHandle();
}


TextureHandle CreateCloudyNightSkyTexture()
{
	LOG_PRINT_ERROR( " Not implemented yet." );
	return TextureHandle();
}


} // namespace amorphous
