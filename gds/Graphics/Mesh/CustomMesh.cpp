#include "CustomMesh.hpp"
#include "CustomMeshRenderer.hpp"
#include "../PrimitiveRenderer.hpp"
#include "../Shader/ShaderManager.hpp"

using namespace std;
//using namespace boost;


Matrix34 GetCameraPoseFromCameraMatrix( const Matrix44& camera_matrix )
{
	Matrix34 camera_pose( Matrix34Identity() );
	camera_pose.matOrient.SetColumn( 0, Vector3( camera_matrix(0,0), camera_matrix(0,1), camera_matrix(0,2) ) );
	camera_pose.matOrient.SetColumn( 1, Vector3( camera_matrix(1,0), camera_matrix(1,1), camera_matrix(1,2) ) );
	camera_pose.matOrient.SetColumn( 2, Vector3( camera_matrix(2,0), camera_matrix(2,1), camera_matrix(2,2) ) );

	const Matrix44 inv_camera_matrix = camera_matrix.GetInverse();
	camera_pose.vPosition.x = inv_camera_matrix(0,3);
	camera_pose.vPosition.y = inv_camera_matrix(1,3);
	camera_pose.vPosition.z = inv_camera_matrix(2,3);

	return camera_pose;
}


inline void ToMatrix34( const Matrix44& src, Matrix34& dest )
{
	dest.matOrient.SetColumn( 0, Vector3( src(0,0), src(1,0), src(2,0) ) );
	dest.matOrient.SetColumn( 1, Vector3( src(0,1), src(1,1), src(2,1) ) );
	dest.matOrient.SetColumn( 2, Vector3( src(0,2), src(1,2), src(2,2) ) );

	dest.vPosition.x = src(0,3);
	dest.vPosition.y = src(1,3);
	dest.vPosition.z = src(2,3);
}


CCustomMesh::VertexColorFormat CCustomMesh::ms_DefaultVertexDiffuseColorFormat = CCustomMesh::VCF_FRGBA;


CCustomMesh::CCustomMesh()
:
m_VertexFlags(0),
m_VertexSize(0),
m_NumUpdatedVertices(0),
m_NumUpdatedIndices(0)
{
	memset( m_ElementOffsets, 0, sizeof(m_ElementOffsets) );
}


void CCustomMesh::SetDiffuseColors( const std::vector<SFloatRGBAColor>& diffuse_colors )
{
	const int num = (int)diffuse_colors.size();
	const int offset = m_ElementOffsets[VEE::DIFFUSE_COLOR];

	if( ms_DefaultVertexDiffuseColorFormat == VCF_ARGB32 )
	{
		for( int i=0; i<num; i++ )
		{
			U32 argb32 = diffuse_colors[i].GetARGB32();
			memcpy( &(m_VertexBuffer[0]) + m_VertexSize * i + offset, &(argb32), sizeof(U32) );
		}
	}
	else if( ms_DefaultVertexDiffuseColorFormat == VCF_FRGBA )
	{
		for( int i=0; i<num; i++ )
			memcpy( &(m_VertexBuffer[0]) + m_VertexSize * i + offset, &(diffuse_colors[i]), sizeof(SFloatRGBAColor) );
	}
}


void CCustomMesh::InitVertexBuffer( int num_vertices, U32 vertex_format_flags )
{
	m_VertexFlags = vertex_format_flags;
	uint vert_size = 0;

	U32 vec3_element_flags[] =
	{
		VFF::POSITION,
		VFF::NORMAL,
		VFF::BINORMAL,
		VFF::TANGENT
	};

	for( int i=0; i<numof(vec3_element_flags); i++ )
	{
		if( vertex_format_flags & vec3_element_flags[i] )
		{
			m_ElementOffsets[VEE::POSITION + i] = vert_size;
			vert_size += sizeof(Vector3);
		}
	}

	// The diffuse color element must be defined before texture coords elements
	if( vertex_format_flags & VFF::DIFFUSE_COLOR )
	{
		if( ms_DefaultVertexDiffuseColorFormat == VCF_ARGB32 )
		{
			m_ElementOffsets[VEE::DIFFUSE_COLOR] = vert_size;
			vert_size += 4;
		}
		else if( ms_DefaultVertexDiffuseColorFormat == VCF_FRGBA )
		{
			m_ElementOffsets[VEE::DIFFUSE_COLOR] = vert_size;
			vert_size += sizeof(float) * 4;
		}
	}

	U32 texcoord2_element_flags[] =
	{
		VFF::TEXCOORD2_0,
		VFF::TEXCOORD2_1,
		VFF::TEXCOORD2_2,
		VFF::TEXCOORD2_3
	};

	for( int i=0; i<numof(texcoord2_element_flags); i++ )
	{
		if( vertex_format_flags & texcoord2_element_flags[i] )
		{
			m_ElementOffsets[VEE::TEXCOORD2_0 + i] = vert_size;
			vert_size += sizeof(TEXCOORD2);
		}
	}

	m_VertexBuffer.resize( vert_size * num_vertices );

	m_VertexSize = vert_size;
}


bool CCustomMesh::LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename, U32 option_flags )
{
	const CMMA_VertexSet& vs = archive.GetVertexSet();
	const int num_verts = vs.GetNumVertices();

	U32 flags = ToVFF(vs.GetVertexFormat());

	InitVertexBuffer( num_verts, flags );

	if( flags & VFF::POSITION )
		SetPositions( vs.vecPosition );

	if( flags & VFF::NORMAL )
		SetNormals( vs.vecNormal );

	if( flags & VFF::TANGENT )
		SetTangents( vs.vecTangent );

	if( flags & VFF::BINORMAL )
		SetBinormals( vs.vecBinormal );

	U32 tex2_flags[] = { VFF::TEXCOORD2_0, VFF::TEXCOORD2_1, VFF::TEXCOORD2_2, VFF::TEXCOORD2_3 };
	for( int i=0; i<4; i++ )
	{
		if( flags & tex2_flags[i] )
			Set2DTexCoords( vs.vecTex[i], i );
	}

	if( flags & VFF::DIFFUSE_COLOR )
		SetDiffuseColors( vs.vecDiffuseColor );

//	InitVertexBuffer( num_vertices, VFF::POSITION|VFF::NORMAL );

	const int num_indices = (int)archive.GetVertexIndex().size();
	InitIndexBuffer( num_indices, sizeof(U16) );

	SetIndices( archive.GetVertexIndex() );

	LoadMaterialsFromArchive( archive, option_flags );

	m_TriangleSets = archive.GetTriangleSet();

	return true;
}


bool CCustomMesh::LoadFromFile( const std::string& mesh_archive_filepath )
{
	C3DMeshModelArchive archive;
	bool loaded = archive.LoadFromFile( mesh_archive_filepath );
	if( !loaded )
		return false;

	m_strFilename = mesh_archive_filepath;

	return LoadFromArchive( archive );
}


void CCustomMesh::Render()
{
	GetCustomMeshRenderer().RenderMesh( *this );
}


void CCustomMesh::Render( CShaderManager& rShaderMgr )
{
	GetCustomMeshRenderer().RenderMesh( *this, rShaderMgr );
}


static bool rear_to_front( const std::pair<float,int>& lhs, const std::pair<float,int>& rhs )
{
	return (rhs.first < lhs.first);
}


void CCustomMesh::RenderZSorted( CShaderManager& rShaderMgr )
{
	if( GetNumVertices() == 0
	 || GetNumIndices() == 0 )
	{
		return;
	}

	const uint num_tris  = GetNumIndices() / 3;
	const uint num_verts = GetNumVertices();

	Vector3 cam_near_plane_n = Vector3(0,0,1);

	const Matrix34 camera_pose = GetCameraPoseFromCameraMatrix( rShaderMgr.GetViewTransform() );
	Vector3 cam_fwd_dir_in_world_space = camera_pose.matOrient.GetColumn(2);
	Matrix34 inv_world_pose( Matrix34Identity() );
	ToMatrix34( rShaderMgr.GetWorldTransform().GetInverse(), inv_world_pose );
	Vector3 cam_fwd_dir = inv_world_pose * cam_fwd_dir_in_world_space;

	static std::vector< std::pair<float,int> > z_and_tri_pairs;
	z_and_tri_pairs.resize( 0 );
	z_and_tri_pairs.reserve( num_tris );

	// sort
	for( uint i=0; i<num_tris; i++ )
	{
		const Vector3 v0 = GetPosition( (int)GetIndex(i*3+0) );
		const Vector3 v1 = GetPosition( (int)GetIndex(i*3+1) );
		const Vector3 v2 = GetPosition( (int)GetIndex(i*3+2) );

		const Vector3 c = (v0 + v1 + v2) / 3.0f;

		float z = Vec3Dot( cam_fwd_dir, c );// - cam_near_plane_d;

		z_and_tri_pairs.push_back( std::pair<float,int>(z,(int)i) );
	}

	std::sort( z_and_tri_pairs.begin(), z_and_tri_pairs.end(), rear_to_front );

	if( GetNumMaterials() == 0 )
		return;

	for( size_t i=0; i<GetMaterial(0).Texture.size(); i++ )
	{
		rShaderMgr.SetTexture( (int)i, GetMaterial(0).Texture[i] );
	}

	CGeneral3DVertex verts[3];

	vector<U16> indices;
	indices.resize( 0 );
	indices.resize( GetNumIndices() );

	for( uint i=0; i<num_tris; i++ )
	{
		const int tri_index = z_and_tri_pairs[i].second;
		for( int j=0; j<3; j++ )
			indices[i*3+j] = GetIndex( tri_index * 3 + j );
//			indices[i*3+j] = GetIndex( i*3+j );
	}

	const size_t index_data_size = sizeof(U16) * indices.size();
	m_ZSortedIndexBuffer.resize( index_data_size );
	memcpy( &m_ZSortedIndexBuffer[0], &indices[0], index_data_size );

	GetCustomMeshRenderer().RenderZSortedMesh( *this, rShaderMgr );
}
