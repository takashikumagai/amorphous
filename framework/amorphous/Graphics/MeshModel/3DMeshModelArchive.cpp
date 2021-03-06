#include "3DMeshModelArchive.hpp"

#include <fstream>

#include "amorphous/Graphics/General3DVertex.hpp"
#include "amorphous/Graphics/TextureUtilities.hpp"
#include "amorphous/Support/Vec3_StringAux.hpp"
#include "amorphous/Support/ImageArchive.hpp"
#include "amorphous/Support/lfs.hpp"


namespace amorphous
{

using namespace serialization;

using namespace std;


//=========================================================================================
// CMMA_VertexSet
//=========================================================================================

void CMMA_VertexSet::Resize( int i )
{
	vecPosition.resize(i);
	vecNormal.resize(i);
	vecBinormal.resize(i);
	vecTangent.resize(i);
	vecDiffuseColor.resize(i);

	int j;
	for( j=0; j<vecTex.size(); j++ )
		vecTex[j].resize(i);

	vecfMatrixWeight.resize(i);
	veciMatrixIndex.resize(i);

/*	for( j=0; j<vecfMatrixWeight.size(); j++ )
		vecfMatrixWeight[j].resize(i);

	for( j=0; j<veciMatrixIndex.size(); j++ )
		veciMatrixIndex[j].resize(i);*/
}


void CMMA_VertexSet::Clear()
{
	SetVertexFormat( 0 );

	vecPosition.clear();
	vecNormal.clear();
	vecBinormal.clear();
	vecTangent.clear();
	vecDiffuseColor.clear();

	int j;
	for( j=0; j<vecTex.size(); j++ )
		vecTex[j].clear();

	vecfMatrixWeight.clear();
	veciMatrixIndex.clear();

/*	for( j=0; j<vecfMatrixWeight.size(); j++ )
		vecfMatrixWeight[j].clear();

	for( j=0; j<veciMatrixIndex.size(); j++ )
		veciMatrixIndex[j].clear();*/
}


void CMMA_VertexSet::GetBlendMatrixIndices_4Bytes( int iVertexIndex, unsigned char *pIndices ) const
{
	int i;
	const TCFixedVector<int,NUM_MAX_BLEND_MATRICES_PER_VERTEX>& rMatIndex = veciMatrixIndex[iVertexIndex];
//	vector<int>& rMatIndex = veciMatrixIndex[iVertexIndex];
	int iNumMatrices = rMatIndex.size();

	for( i=0; i<iNumMatrices; i++ )
	{
		if( rMatIndex[i] < 0 ) pIndices[i] = 0;
		if( 255 < rMatIndex[i] ) pIndices[i] = 255;
		else pIndices[i] = (unsigned char)rMatIndex[i];
	}

	for( ; i<4; i++ )
	{
		pIndices[i] = 0;
	}
}


void CMMA_VertexSet::GetBlendMatrixWeights_4Floats( int iVertexIndex, float *pWeight ) const
{
	int i;
	const TCFixedVector<float,NUM_MAX_BLEND_MATRICES_PER_VERTEX>& rMatWeight = vecfMatrixWeight[iVertexIndex];
//	vector<float>& rMatWeight = vecfMatrixWeight[iVertexIndex];
	int iNumMatrices = rMatWeight.size();

	for( i=0; i<iNumMatrices; i++ )
	{
		pWeight[i] = rMatWeight[i];
	}

	for( ; i<4; i++ )
	{
		pWeight[i] = 0;
	}
}


void CMMA_VertexSet::GetVertices( vector<General3DVertex>& dest_buffer ) const
{
	dest_buffer.resize( GetNumVertices() );

	size_t i, num_verts = GetNumVertices();
	if( m_VertexFormatFlag & VF_POSITION )
	{
		for( i=0; i<num_verts; i++ )
			dest_buffer[i].m_vPosition = vecPosition[i];
	}

	if( m_VertexFormatFlag & VF_NORMAL )
	{
		for( i=0; i<num_verts; i++ )
			dest_buffer[i].m_vNormal = vecNormal[i];
	}

	if( m_VertexFormatFlag & VF_DIFFUSE_COLOR )
	{
		for( i=0; i<num_verts; i++ )
			dest_buffer[i].m_DiffuseColor = vecDiffuseColor[i];
	}

	if( m_VertexFormatFlag & VF_BUMPMAP )
	{
		for( i=0; i<num_verts; i++ )
		{
			dest_buffer[i].m_vBinormal = vecBinormal[i];
			dest_buffer[i].m_vTangent  = vecTangent[i];
		}
	}

	if( m_VertexFormatFlag & VF_2D_TEXCOORD )
	{
		for( i=0; i<num_verts; i++ )
		{
			dest_buffer[i].m_TextureCoord.resize( 1 );
			dest_buffer[i].m_TextureCoord[0] = vecTex[0][i];
		}
	}

	if( m_VertexFormatFlag & VF_WEIGHT )
	{
		for( i=0; i<num_verts; i++ )
		{
//			dest_buffer[i].m_fMatrixWeight = ;
//			dest_buffer[i].m_fMatrixWeight = ;
		}
	}

/*	if( m_VertexFormatFlag & VF_POSITION )
	{
		for( i=0; i<num_verts; i++ )
			dest_buffer[i]. = ;
	}*/
}


void CMMA_VertexSet::Serialize( IArchive& ar, const unsigned int version )
{
	ar & m_VertexFormatFlag;

	ar & vecPosition;

	ar & vecNormal;

	ar & vecBinormal;
	ar & vecTangent;

	ar & vecDiffuseColor;

	ar & vecTex;

	ar & vecfMatrixWeight;
	ar & veciMatrixIndex;
}


//=========================================================================================
// CMMA_TriangleSet
//=========================================================================================

CMMA_TriangleSet::CMMA_TriangleSet()
:
m_iStartIndex(0),
m_iMinIndex(0),
m_iNumVertexBlocksToCover(0),
m_iNumTriangles(0),
m_AABB( AABB3(Vector3(0,0,0),Vector3(0,0,0)) )
{}


void CMMA_TriangleSet::Serialize( IArchive& ar, const unsigned int version )
{
	ar & m_iStartIndex;
	
	ar & m_iMinIndex;

	ar & m_iNumVertexBlocksToCover;

	ar & m_iNumTriangles;

	if( 1 <= version )
        ar & m_AABB;
}



void CMMA_TriangleSet::DumpToText( std::string& dest ) const
{
	dest += fmt_string( "start index: %d\n",   m_iStartIndex );
	dest += fmt_string( "min index: %d\n",     m_iMinIndex );
	dest += fmt_string( "num triangles: %d\n", m_iNumTriangles );
	dest += fmt_string( "num vertex blocks to cover: %d\n", m_iNumVertexBlocksToCover );
	dest += fmt_string( "aabb: %s", to_string(m_AABB).c_str() );
}

//=========================================================================================
// CMMA_Texture
//=========================================================================================

void CMMA_Texture::Serialize( IArchive& ar, const unsigned int version )
{
	ar & type;
	ar & strFilename;
	ar & vecTexelData;
	ar & vecfTexelData;
}


//=========================================================================================
// CMMA_Material
//=========================================================================================

void CMMA_Material::Serialize( IArchive& ar, const unsigned int version )
{
	if( 2 <= version )
		ar & Name;

	ar & fSpecular;

	if( 4 <= version )
		ar & m_Params;

	if( version < 1 )
		assert( !string( string(__FUNCTION__) + " : archive version outdated - no longer supported" ).c_str() );

	if( 5 <= version )
		ar & vecTexture;
	else
	{
		// Assumes ar.GetMode() == IArchive::MODE_INPUT
		vector<CMMA_Texture> tex;
		ar & tex;

		vecTexture.resize( tex.size() );
		for( size_t i=0; i<tex.size(); i++ )
		{
			vecTexture[i].ResourcePath = tex[i].strFilename;
			if( tex[i].type == CMMA_Texture::SINGLECOLOR && 0 < tex[i].vecfTexelData.size_x() )
			{
				vecTexture[i].Format = TextureFormat::A8R8G8B8;
				vecTexture[i].Width  = 1;
				vecTexture[i].Height = 1;
				vecTexture[i].pLoader.reset( new SingleColorTextureGenerator( tex[i].vecfTexelData(0,0) ) );
			}
		}
	}

	if( 3 <= version )
		ar & fMinVertexDiffuseAlpha;
}


void CMMA_Material::SetDefault()
{
	fSpecular = 0;
	vecTexture.resize( 0 );

	fMinVertexDiffuseAlpha = 1.0f;

//	strSurfaceTexture = "";
//	strNormalMapTexture = "";
}


//=========================================================================================
// CMMA_Bone
//=========================================================================================

int CMMA_Bone::GetNumBones_r() const
{
	int num = 0;
	for( size_t i=0; i<vecChild.size(); i++ )
	{
		num += vecChild[i].GetNumBones_r();
	}

	return num + 1;
}


void CMMA_Bone::Serialize( IArchive& ar, const unsigned int version )
{
	ar & strName;
	ar & vLocalOffset;
	ar & BoneTransform;
	ar & vecChild;
}



//void AddTexturesToBinaryDatabase( C3DMeshModelArchive& mesh_archive, ... ) /// error: must specify the namespace. See below

void AddTexturesToBinaryDatabase( C3DMeshModelArchive& mesh_archive,
								  const string& db_filepath,
								  CBinaryDatabase<string> &db,
								  bool bUseTextureBasenameForKey )
{
	const size_t num_materials = mesh_archive.GetMaterial().size();
	for( size_t i=0; i<num_materials; i++ )
	{
		const size_t num_textures = mesh_archive.GetMaterial()[i].vecTexture.size();
		for( size_t j=0; j<num_textures; j++ )
		{
			TextureResourceDesc& rTexture = mesh_archive.GetMaterial()[i].vecTexture[j];

			const string tex_filename = rTexture.ResourcePath;

			if( tex_filename.length() == 0 )
				continue;

			if( tex_filename.find("::") != string::npos )
			{
				LOG_PRINT( " - The texture filepath already includes db & keyname separator '::'. Skipping the process for" + tex_filename );
				continue;
			}

			string tex_key;
			
			if( bUseTextureBasenameForKey )
				tex_key = lfs::get_leaf(tex_filename);
			else
				tex_key = tex_filename;

	//		string tex_resource_name = db_filename + "::" + tex_key;
			string tex_resource_name = db_filepath + "::" + tex_key;

			// overwrite original texture filename with the resource name
			// - (db filename) + "::" + (archive key name)
			rTexture.ResourcePath = tex_resource_name;

			if( db.KeyExists(tex_key) )
			{
				// the texture file has been already saved to database
				// - skip this texture
				continue;
			}

			ImageArchive img_archive( tex_filename );

			if( img_archive.IsValid() )
			{
				// add image archive to db
				db.AddData( tex_key, img_archive );
			}
			else
				LOG_PRINT_ERROR( " - invalid texture filepath: " + tex_filename );
		}
	}
}


Result::Name CreateSingleSubsetMeshArchive(
	const vector<Vector3>& positions,
	const vector<Vector3>& normals,
	const vector<SFloatRGBAColor>& diffuse_colors,
	const vector<TEXCOORD2>& tex_coords,
	const vector< vector<unsigned int> >& polygons,
	C3DMeshModelArchive& dest_mesh
	)
{
	if( positions.empty() )
	{
		return Result::INVALID_ARGS;
	}

	CMMA_VertexSet& vs = dest_mesh.GetVertexSet();

	vs.Clear();

	vs.vecPosition     = positions;
	vs.vecNormal       = normals;
	vs.vecDiffuseColor = diffuse_colors;

	if( 0 < tex_coords.size() )
	{
		vs.vecTex.resize( 1 );
		vs.vecTex[0] = tex_coords;
	}

	vs.SetVertexFormat(
		CMMA_VertexSet::VF_POSITION
		| ( (0 < normals.size())        ? CMMA_VertexSet::VF_NORMAL        : 0 )
		| ( (0 < diffuse_colors.size()) ? CMMA_VertexSet::VF_DIFFUSE_COLOR : 0 )
		| ( (0 < tex_coords.size())     ? CMMA_VertexSet::VF_2D_TEXCOORD0  : 0 )
		);

	dest_mesh.GetVertexIndex().resize( 0 );
	dest_mesh.GetVertexIndex().reserve( polygons.size() * 3 );

	const int num_polygons = (int)polygons.size();
	for( int i=0; i<num_polygons; i++ )
	{
		int num_polygon_vertices = (int)polygons[i].size();
		for( int j=1; j<num_polygon_vertices-1; j++ )
		{
			dest_mesh.GetVertexIndex().push_back( polygons[i][0] );
			dest_mesh.GetVertexIndex().push_back( polygons[i][j] );
			dest_mesh.GetVertexIndex().push_back( polygons[i][j+1] );
		}
	}

	dest_mesh.GetMaterial().resize( 1 );
	dest_mesh.GetMaterial()[0].vecTexture.resize( 1 );
//	dest_mesh.GetMaterial()[0].vecTexture[0].strFilename = "default.png";
//	dest_mesh.GetMaterial()[0].vecTexture[0].type = CMMA_Texture::SINGLECOLOR;
//	dest_mesh.GetMaterial()[0].vecTexture[0].vecfTexelData.resize( 1, 1, SFloatRGBAColor::White() );
	SetSingleColorTextureDesc(
		dest_mesh.GetMaterial()[0].vecTexture[0],
		SFloatRGBAColor::White(),
		TextureFormat::A8R8G8B8,
		1, // texture width
		1  // texture height
		);

	vector<CMMA_TriangleSet>& triangle_sets = dest_mesh.GetTriangleSet();
	triangle_sets.resize( 1 );
	triangle_sets[0].m_iStartIndex             = 0;
	triangle_sets[0].m_iMinIndex               = 0;
	triangle_sets[0].m_iNumVertexBlocksToCover = (int)positions.size();
	triangle_sets[0].m_iNumTriangles           = (int)dest_mesh.GetVertexIndex().size() / 3;

	dest_mesh.UpdateAABBs();

	return Result::SUCCESS;
}


//=========================================================================================
// C3DMeshModelArchive
//=========================================================================================

C3DMeshModelArchive::C3DMeshModelArchive()
{
	m_AABB.Nullify();
}


C3DMeshModelArchive::~C3DMeshModelArchive()
{
}


void C3DMeshModelArchive::Serialize( IArchive& ar, const unsigned int version )
{
	if( version < 2 )
		assert( !string( string(__FUNCTION__) + " : archive version outdated - no longer supported" ).c_str() );

	ar & m_VertexSet;

	ar & m_vecVertexIndex;

	ar & m_vecMaterial;
	ar & m_vecTriangleSet;

	ar & m_SkeletonRootBone;

	ar & m_AABB;
}


void C3DMeshModelArchive::SetMaterial( unsigned int index, CMMA_Material& rMaterial )
{
	if( m_vecMaterial.size() <= index )
		m_vecMaterial.resize( index + 1 );

	m_vecMaterial[index] = rMaterial;
}


void C3DMeshModelArchive::UpdateAABBs()
{
	m_AABB.Nullify();

	// create aabb for each triangle set
	size_t i, num_triangle_sets = m_vecTriangleSet.size();
	for( i=0; i<num_triangle_sets; i++ )
	{
		CMMA_TriangleSet& rTriSet = m_vecTriangleSet[i];
		const vector<Vector3>& vecVertPosition = m_VertexSet.vecPosition;
		const vector<unsigned int>& index_buffer = m_vecVertexIndex;

		AABB3 aabb;
		aabb.Nullify();
		size_t offset = rTriSet.m_iStartIndex;
		size_t j, num_triangles = rTriSet.m_iNumTriangles;
		for( j=0; j<num_triangles*3; j++ )
		{
			aabb.AddPoint( vecVertPosition[index_buffer[offset+j]] );
		}

		rTriSet.m_AABB = aabb;

		m_AABB.MergeAABB( aabb );
	}
}


void C3DMeshModelArchive::UpdateMinimumVertexDiffuseAlpha()
{
	if( m_VertexSet.vecDiffuseColor.size() == 0 )
		return; // the mesh vertices do not have diffuse colors

	const vector<SFloatRGBAColor>& vecDiffuseColor = m_VertexSet.vecDiffuseColor;

	float min_alpha;
	const size_t num_materials = m_vecMaterial.size();
	for( size_t i=0; i<num_materials; i++ )
	{
		CMMA_TriangleSet& rTriSet = m_vecTriangleSet[i];

		min_alpha = 1.0f;
		float alpha = min_alpha;
		const size_t start_index = rTriSet.m_iStartIndex;
		const size_t end_index   = rTriSet.m_iStartIndex + rTriSet.m_iNumTriangles * 3;
		for( size_t j=start_index; j<end_index; j++ )
		{
			alpha = vecDiffuseColor[ m_vecVertexIndex[j] ].alpha;
			if( alpha < min_alpha )
				min_alpha = alpha;
		}

		m_vecMaterial[i].fMinVertexDiffuseAlpha = min_alpha;
	}
}


void C3DMeshModelArchive::Scale( float factor )
{
	CMMA_VertexSet& vertex_set = GetVertexSet();
	vector<Vector3>& vecPosition = vertex_set.vecPosition;

//	int num_vertices = vertex_set.GetNumVertices();
	size_t i, num_vertices = vecPosition.size();

	for( i=0; i<num_vertices; i++ )
	{
		vecPosition[i] *= factor;
	}
}


void C3DMeshModelArchive::FlipTriangles()
{
	if( m_vecVertexIndex.size() % 3 != 0 )
		return;

	CMMA_VertexSet& vertex_set = GetVertexSet();

	if( vertex_set.GetVertexFormat() & CMMA_VertexSet::VF_NORMAL )
	{
		for( size_t i=0; i<vertex_set.vecNormal.size(); i++ )
			vertex_set.vecNormal[i] *= -1.0f;
	}

	const int num_triangles = (int)m_vecVertexIndex.size() / 3;
	for( int i=0; i<num_triangles; i++ )
		std::swap( m_vecVertexIndex[i*3], m_vecVertexIndex[i*3+2] );
}


#define INVALID_POINT_REPRESENTATIVE	65535

void C3DMeshModelArchive::GeneratePointRepresentatives( vector<unsigned short>& rvecusPtRep )
{
	int i, j;
	const int iNumVeritices = m_VertexSet.GetNumVertices();

	vector<Vector3>& vecvPosition = m_VertexSet.vecPosition;

	rvecusPtRep.resize( iNumVeritices, INVALID_POINT_REPRESENTATIVE );

	for( i=0; i<iNumVeritices; i++ )
	{
		if( rvecusPtRep[i] != INVALID_POINT_REPRESENTATIVE )
			continue;

		rvecusPtRep[i] = i;

		for( j=i+1; j<iNumVeritices; j++ )
		{
			if( vecvPosition[i] == vecvPosition[j] )
				rvecusPtRep[j] = i;
		}

	}
}


void C3DMeshModelArchive::CopySkeletonFrom( C3DMeshModelArchive& rMesh )
{
	m_SkeletonRootBone = rMesh.m_SkeletonRootBone;
}


void C3DMeshModelArchive::WriteToTextFile( const string& filename )
{
//	fstream f;	f.open( filename, ios::out );

	FILE *fp = fopen( filename.c_str(), "w" );

	if( !fp )
	{
		LOG_PRINT_ERROR( " Failed to open the file: " + filename );
		return;
	}

	int i, iNumVeritices = m_VertexSet.GetNumVertices();

	fprintf( fp, "%d vertices\n", iNumVeritices );

	fprintf( fp, "============== POSITION ==============\n" );

	for( i=0; i<iNumVeritices; i++ )
	{
		Vector3& vPos = m_VertexSet.vecPosition[i];
		fprintf( fp, "%.3f %.3f %.3f\n", vPos.x, vPos.y, vPos.z );
//		f << vPos.x << vPos.y << vPos.z;
	}


	if( 0 < m_VertexSet.vecNormal.size() )
	{
		fprintf( fp, "============== NORMAL ==============\n" );

		for( i=0; i<iNumVeritices; i++ )
		{
			Vector3& vNormal = m_VertexSet.vecNormal[i];
			fprintf( fp, "%.3f %.3f %.3f\n", vNormal.x, vNormal.y, vNormal.z );
		}
	}

	if( m_VertexSet.GetVertexFormat() & CMMA_VertexSet::VF_DIFFUSE_COLOR )
	{
		fprintf( fp, "============== VERTEX COLOR (RGBA) ==============\n" );

		for( i=0; i<iNumVeritices; i++ )
		{
			const SFloatRGBAColor& color = m_VertexSet.vecDiffuseColor[i];
			fprintf( fp, "%.2f %.2f %.2f %.2f\n", color.red, color.green, color.blue, color.alpha );
		}
	}

	if( 0 < m_VertexSet.vecBinormal.size() )
	{
		fprintf( fp, "============== BINORMAL ==============\n" );

		for( i=0; i<iNumVeritices; i++ )
		{
			const Vector3& vBinormal = m_VertexSet.vecBinormal[i];
			fprintf( fp, "%.3f %.3f %.3f\n", vBinormal.x, vBinormal.y, vBinormal.z );
		}
	}

	if( 0 < m_VertexSet.vecTangent.size() )
	{
		fprintf( fp, "============== TANGENT ==============\n" );

		for( i=0; i<iNumVeritices; i++ )
		{
			const Vector3& vTangent = m_VertexSet.vecTangent[i];
			fprintf( fp, "%.3f %.3f %.3f\n", vTangent.x, vTangent.y, vTangent.z );
		}
	}

	if( m_VertexSet.GetVertexFormat() & CMMA_VertexSet::VF_WEIGHT )
	{
		fprintf( fp, "============== VERTEX BLEND WEIGHT ==============\n" );
		fprintf( fp, "index[0],  index[1],  index[2],  index[3]...\n" );
		fprintf( fp, "weight[0], weight[1], weight[2], weight[3]...\n" );

		int j;
		for( i=0; i<iNumVeritices; i++ )
		{
			TCFixedVector<int,CMMA_VertexSet::NUM_MAX_BLEND_MATRICES_PER_VERTEX> weight_index = m_VertexSet.veciMatrixIndex[i];
			TCFixedVector<float,CMMA_VertexSet::NUM_MAX_BLEND_MATRICES_PER_VERTEX> weight = m_VertexSet.vecfMatrixWeight[i];

			for( j=0; j<weight_index.size(); j++ )	{ fprintf( fp, "%02d", weight_index[j] );	if( j<weight_index.size()-1 ) fprintf( fp, ", " ); }
			fprintf( fp, "\n" );
			for( j=0; j<weight.size(); j++ )		{ fprintf( fp, "%.3f", weight[j] );			if( j<weight.size()-1 ) fprintf( fp, ",    " ); }
			fprintf( fp, "\n" );
		}
	}
/*
	for( i=0; i<iNumVeritices; i++ )
	{
		Vector3& vBinormal = m_VertexSet.vecBinormal[i];
		fprintf( fp, "%.3f %.3f %.3f\n", vBinormal.x, vBinormal.y, vBinormal.z );
	}

	for( i=0; i<iNumVeritices; i++ )
	{
		Vector3& vPos = m_VertexSet.vecPosition[i];
		fprintf( fp, "%.3f %.3f %.3f\n", vPos.x, vPos.y, vPos.z );
	}*/

	int iNumTris;
	const int num_materials = (int)m_vecTriangleSet.size();
	int offset, tri;

	fprintf( fp, "\n\n" );
	fprintf( fp, "%d materials\n", num_materials );
	fprintf( fp, "total %zd triangles\n\n", m_vecVertexIndex.size() / 3 );

	vector<unsigned int>& rvecIndex = m_vecVertexIndex;
	for( i=0; i<num_materials; i++ )
	{
		fprintf( fp, "material[%d]\n", i );

		fprintf( fp, "name: %s\n", m_vecMaterial[i].Name.c_str() );

		const size_t num_textures = m_vecMaterial[i].vecTexture.size();
		for( size_t tex=0; tex<num_textures; tex++ )
			fprintf( fp, "texture[%zd]: \"%s\"\n", tex, m_vecMaterial[i].vecTexture[tex].ResourcePath.c_str() );

		fprintf( fp, "min. vertex alpha: %f\n", m_vecMaterial[i].fMinVertexDiffuseAlpha );

//		fprintf( fp, "surface texture:    \"%s\"\n", m_vecMaterial[i].SurfaceTexture.strFilename.c_str() );
//		fprintf( fp, "normal map texture: \"%s\"\n", m_vecMaterial[i].NormalMapTexture.strFilename.c_str() );

		iNumTris = m_vecTriangleSet[i].m_iNumTriangles;
		offset = m_vecTriangleSet[i].m_iStartIndex;

		fprintf( fp, "%d triangles\n", iNumTris );

		for( tri=0; tri<iNumTris; tri++ )
		{
			fprintf( fp, "%d ",  m_vecVertexIndex[offset + tri * 3] );
			fprintf( fp, "%d ",  m_vecVertexIndex[offset + tri * 3 + 1] );
			fprintf( fp, "%d\n", m_vecVertexIndex[offset + tri * 3 + 2] );
		}
        fprintf( fp, "\n" );
	}

	fprintf( fp, "\n============== TEXTURE COORDINATES ==============\n" );

	if( 0 < m_VertexSet.vecTex.size() )
	{
		for( i=0; i<iNumVeritices; i++ )
		{
			TEXCOORD2& t = m_VertexSet.vecTex[0][i];
			fprintf( fp, "%.3f %.3f\n", t.u, t.v );
		}
	}

	fprintf( fp, "\n### triangle sets ###\n" );

	const int num_triangle_sets = (int)m_vecTriangleSet.size();
	for( i=0; i<num_triangle_sets; i++ )
	{
		CMMA_TriangleSet& tri_set = m_vecTriangleSet[i];
		fprintf( fp, "[%02d]-------------------------\n", i );
		fprintf( fp, "start index:  %d\n", tri_set.m_iStartIndex );
		fprintf( fp, "triangles:    %d\n", tri_set.m_iNumTriangles );
		fprintf( fp, "min index:    %d\n", tri_set.m_iMinIndex );
		fprintf( fp, "vert. blocks: %d\n", tri_set.m_iNumVertexBlocksToCover	);
		fprintf( fp, "aabb:         %s\n", to_string(tri_set.m_AABB).c_str() );
	}

	if( 0 < m_SkeletonRootBone.vecChild.size() )
	{
		fprintf( fp, "\n============== SKELETON ==============\n" );
		const int num_bones = m_SkeletonRootBone.GetNumBones_r();
		fprintf( fp, "%d bone%s in total\n", num_bones, 1 < num_bones ? "s" : "" );
//		m_SkeletonRootBone.WriteToTextFile( fp );
	}

	fclose(fp);
}


} // namespace amorphous
