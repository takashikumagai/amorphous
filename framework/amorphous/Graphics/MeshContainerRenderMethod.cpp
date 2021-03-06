#include "MeshContainerRenderMethod.hpp"
#include "MeshObjectContainer.hpp"
#include "amorphous/Support/Serialization/Serialization_BoostSmartPtr.hpp"
#include "amorphous/XML/XMLNode.hpp"
#include "amorphous/XML/LoadFromXMLNode_3DMath.hpp"
#include "amorphous/Graphics/Shader/ShaderManager.hpp"
#include "amorphous/Graphics/Shader/Serialization_ShaderTechniqueHandle.hpp"
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"


namespace amorphous
{

using namespace std;


/*
Defines a method to render the meshes in a mesh container node
XML sample for MeshContainerNode GameItem::m_MeshContainerRootNode


- A simple render method to render a single mesh with a single shader technique

<MeshRenderMethod>
	<MeshContainerNodeRenderMethod>
		<MeshContainerRenderMethod>
			<MeshRenderMethod lod="0">
				<ShaderFilepath>simple.fx</ShaderFilepath>
				<Technique>Simple</Technique>
			</MeshRenderMethod>
		</MeshContainerRenderMethod>
	</MeshContainerNodeRenderMethod>
</MeshRenderMethod>

- render method to render a mesh with different shaders and shader techniques for each subset
  - Note that <SubsetRenderMethod> is used instead of <MeshRenderMethod>,
    and <Name> nodes are added at every <SubsetRenderMethod> node
<MeshRenderMethod>
	<MeshContainerNodeRenderMethod>
		<MeshContainerRenderMethod>
			<SubsetRenderMethod lo>
				<Name>RubberParts</Name> << name of the subset to render with this technique
				<ShaderFilepath>Rubber.fx</ShaderFilepath>
				<Technique>Rubber</Technique>
			</SubsetRenderMethod>
			<SubsetRenderMethod>
				<Name>MetallicParts</Name>
				<ShaderFilepath>Metal.fx</ShaderFilepath>
				<Technique>Metal</Technique>
			</SubsetRenderMethod>
		</MeshContainerRenderMethod>
	</MeshContainerNodeRenderMethod>
</MeshRenderMethod>

<MeshRenderMethod>
	<MeshNode>
		<!-- Mesh node contains one or more 'mesh units'-->
		<MeshUnit>
			<MeshContainer>
				<SubsetsToRender>
					<Name>Eyes</Name>
					<Name>Cloths</Name>
				<SubsetsToRender>
				<Shader>
			</MeshContainer>
		</MeshUnit>
		<MeshUnit>
			<MeshContainer>
				
			</MeshContainer>
			<LocalPose>
				<Heading>60</Heading>
				<Pitch>60</Pitch>
				<Position>0 0 0</Position>
			</LocalPose>
		</MeshUnit>

		<!-- Child mesh noes -->
		<MeshNode>

		</MeshNode>
	</MeshNode>
</MeshRenderMethod>


*/



/*
--------------------------------------------------
 XML format for shader technique
--------------------------------------------------

no LOD, single technique

<SingleShaderTechnique>HSLighting</SingleShaderTechnique>


no LOD, multiple materials (Not implemented yet)

<ShaderTechniques>
	<Material index="0">EnvMap</Material>
	<Material index="1">HSLighting</Material>
	<Material index="2">HSLighting</Material>
</ShaderTechniques>


LOD, multiple materials (Not implemented yet)

<ShaderTechniques>
	<Material index="0">
		<High>EnvMap</High>
		<Mid>EnvMap</Mid>
		<Low>Default</Low>
	</Material>
	<Material index="1">
		<High>HSLighting</High>
		<Mid>HSLighting</Mid>
		<Low>Default</Low>
	</Material>
	<Material index="2">
		<High>HSLighting</High>
		<Mid>HSLighting</Mid>
		<Low>Default</Low>
	</Material>
</ShaderTechniques>

*/




//  old text desc file (deprecated. Use the XML format instead)
/*

How to load from text file?

text file
MeshContainer.MeshObjectFilename	plane.msh

begin: MeshContainer
	filename	models/plane.msh
	extra_tex	0 textures/specular.msh
	shader_tech 0 VertexBlend_1HSDL_Specular_CTS  SingleHSDL_Specular_CTS     VertLights
	shader_tech 1 SpecularMembrane_1HSDL_CTS      SpecularMembrane_1HSDL_CTS  VertLights
end: MeshContainer
*/


bool SubsetRenderMethod::Load()
{
	bool loaded = false;
	for( size_t i=0; i<m_vecpShaderParamsLoader.size(); i++ )
	{
		if( m_vecpShaderParamsLoader[i] )
			loaded = m_vecpShaderParamsLoader[i]->LoadResource();
	}

	return m_Shader.Load( m_ShaderDesc );
}


void SubsetRenderMethod::LoadFromXMLNode( XMLNode& reader )
{
	reader.GetChildElementTextContent( "ShaderFilepath", m_ShaderDesc.ResourcePath );

	string tech_name;
	reader.GetChildElementTextContent( "Technique", tech_name );
	m_Technique.SetTechniqueName( tech_name.c_str() );
}


void SubsetRenderMethod::Serialize( IArchive& ar, const unsigned int version )
{
	ar & m_ShaderDesc;
	ar & m_Technique;
}


//static int sg_iPrevShaderManagerID = -1;


void MeshContainerRenderMethod::RenderMeshOrMeshSubsets( BasicMesh &mesh,
	                                                      const vector<int>& subset_indices,
//														  ShaderManager& shader_mgr,
														  SubsetRenderMethod& render_method,
														  const Matrix34& world_transform )
{
	// Render with a single shader & a single technique 
//	ShaderManager *pShaderMgr = m_vecMeshRenderMethod[lod_index].m_Shader.GetShaderManager();
	ShaderManager *pShaderMgr = render_method.m_Shader.GetShaderManager();
	if( !pShaderMgr )
		return;

	ShaderManager& shader_mgr = (*pShaderMgr);

	shader_mgr.SetWorldTransform( world_transform );

/*	if( true )//sg_iPrevShaderManagerID != pShaderMgr->GetShaderManagerID() )
	{
		for( size_t i=0; i<vecpShaderParamsWriter.size(); i++ )
			vecpShaderParamsWriter[i]->UpdateShaderParams( *pShaderMgr );
	}*/

//	SubsetRenderMethod& render_method = m_vecMeshRenderMethod[lod_index];

	// update shader params
	for( size_t i=0; i<render_method.m_vecpShaderParamsLoader.size(); i++ )
	{
		render_method.m_vecpShaderParamsLoader[i]->UpdateShaderParams( shader_mgr );
	}

	// render
	Result::Name res = shader_mgr.SetTechnique( render_method.m_Technique );
	if( subset_indices.size() == 0 )
	{
		// Render all the mesh subsets with a single shader & a single technique 
		mesh.Render( shader_mgr );
	}
	else
	{
		// render only the specified subsets with a single shader & a single technique
		for( int i=0; i<(int)subset_indices.size(); i++ )
		{
			mesh.RenderSubset( shader_mgr, subset_indices[i] );
		}
	}

	// reset shader params if necessary
	for( size_t i=0; i<render_method.m_vecpShaderParamsLoader.size(); i++ )
	{
		render_method.m_vecpShaderParamsLoader[i]->ResetShaderParams( shader_mgr );
	}
}


// - Set the world transform 'world_transform' to shader
// - Update shader params
// - Set technique
void MeshContainerRenderMethod::RenderMesh( BasicMesh &mesh, const Matrix34& world_transform )
{
	if( !m_RenderMethodsAndSubsetIndices.empty() )
	{
		if( m_RenderMethodsAndSubsetIndices.size() == 1
		 && m_RenderMethodsAndSubsetIndices[0].second.empty() )
		{
			// render all the subsets at once
			RenderMeshOrMeshSubsets( mesh, m_RenderMethodsAndSubsetIndices[0].second, m_RenderMethodsAndSubsetIndices[0].first, world_transform );
		}
		else
		{
			for( int i=0; i<(int)m_RenderMethodsAndSubsetIndices.size(); i++ )
			{
				SubsetRenderMethod& render_method = m_RenderMethodsAndSubsetIndices[i].first;
				const vector<int>& subset_indices  = m_RenderMethodsAndSubsetIndices[i].second;

				for( int j=0; j<(int)subset_indices.size(); j++ )
				{
					RenderMeshOrMeshSubsets( mesh, subset_indices, render_method, world_transform );
				}
			}
		}
	}
	else if( 0 < m_vecSubsetNameToRenderMethod.size() )
	{
		// render subsets one by one

		// set different shaders / techniques for each subset
		const int num_subsets = mesh.GetNumMaterials();
		std::vector<int> *pvecIndicesOfSubsetsToRender = nullptr;
		if( m_vecIndicesOfSubsetsToRender.size() == 0 )
		{
			// render all the subsets
			// - create the full indices list
			// - For the same mesh, this is done only once.
			for( int j=(int)m_vecFullIndicesOfSubsets.size(); j<num_subsets; j++ )
				m_vecFullIndicesOfSubsets.push_back( j );
			pvecIndicesOfSubsetsToRender = &m_vecFullIndicesOfSubsets;
		}
		else
		{
			pvecIndicesOfSubsetsToRender = &m_vecIndicesOfSubsetsToRender;
		}

		int lod_index = 0;
//		for( i=0; i<num_subsets; i++ )
		for( size_t i=0; i<pvecIndicesOfSubsetsToRender->size(); i++ )
		{
			int index = (*pvecIndicesOfSubsetsToRender)[i];

			map< string, SubsetRenderMethod >::iterator itr
				= m_vecSubsetNameToRenderMethod[lod_index].find( mesh.GetMaterial(index).Name );

			if( itr == m_vecSubsetNameToRenderMethod[lod_index].end() )
				continue;

			SubsetRenderMethod& subset_render_method = (*itr).second;

			ShaderManager *pShaderMgr = subset_render_method.m_Shader.GetShaderManager();
			if( !pShaderMgr )
				continue;

			pShaderMgr->SetWorldTransform( world_transform );

			pShaderMgr->SetTechnique( subset_render_method.m_Technique );

			for( size_t j=0; j<subset_render_method.m_vecpShaderParamsLoader.size(); j++ )
			{
				subset_render_method.m_vecpShaderParamsLoader[j]->UpdateShaderParams( *pShaderMgr );
			}

			mesh.RenderSubset( *pShaderMgr, index );

			for( size_t j=0; j<subset_render_method.m_vecpShaderParamsLoader.size(); j++ )
			{
				subset_render_method.m_vecpShaderParamsLoader[j]->ResetShaderParams( *pShaderMgr );
			}
		}
	}
}


void MeshContainerRenderMethod::SetShaderParamsLoaderToAllMeshRenderMethods( std::shared_ptr<ShaderParamsLoader> pShaderParamsLoader )
{
	for( size_t i=0; i<m_RenderMethodsAndSubsetIndices.size(); i++ )
	{
		m_RenderMethodsAndSubsetIndices[i].first.m_vecpShaderParamsLoader.push_back( pShaderParamsLoader );
	}

//	typedef pair<string,SubsetRenderMethod> str_rendermethod;
	for( size_t i=0; i<m_vecSubsetNameToRenderMethod.size(); i++ )
	{
		map<string,SubsetRenderMethod>::iterator itr;
		for( itr = m_vecSubsetNameToRenderMethod[i].begin();
			 itr != m_vecSubsetNameToRenderMethod[i].end();
			 itr++ )
		{
			itr->second.m_vecpShaderParamsLoader.push_back( pShaderParamsLoader );
		}
/*		for( str_rendermethod& p : m_vecSubsetNameToRenderMethod[i] ) // error
		{
			p.second.m_vecpShaderParamsLoader.push_back( pShaderParamsLoader );
		}*/
	}
}


void MeshContainerRenderMethod::RemoveShaderParamsLoaderFromAllMeshRenderMethods( std::shared_ptr<ShaderParamsLoader> pShaderParamsLoader )
{
	for( size_t i=0; i<m_RenderMethodsAndSubsetIndices.size(); i++ )
	{
		vector< shared_ptr<ShaderParamsLoader> >::iterator itrParamsLoader
			= m_RenderMethodsAndSubsetIndices[i].first.m_vecpShaderParamsLoader.begin();
		while( itrParamsLoader != m_RenderMethodsAndSubsetIndices[i].first.m_vecpShaderParamsLoader.end() )
		{
			if( (*itrParamsLoader) == pShaderParamsLoader )
			{
				itrParamsLoader = m_RenderMethodsAndSubsetIndices[i].first.m_vecpShaderParamsLoader.erase( itrParamsLoader );
			}
			else
				itrParamsLoader++;
		}
	}

//	typedef pair<string,SubsetRenderMethod> str_rendermethod;
	for( size_t i=0; i<m_vecSubsetNameToRenderMethod.size(); i++ )
	{
		map<string,SubsetRenderMethod>::iterator itr;
		for( itr = m_vecSubsetNameToRenderMethod[i].begin();
			 itr != m_vecSubsetNameToRenderMethod[i].end();
			 itr++ )
		{
			vector< shared_ptr<ShaderParamsLoader> >::iterator itrParamsLoader
				= itr->second.m_vecpShaderParamsLoader.begin();
			while( itrParamsLoader != itr->second.m_vecpShaderParamsLoader.end() )
			{
				if( (*itrParamsLoader) == pShaderParamsLoader )
				{
					itrParamsLoader = itr->second.m_vecpShaderParamsLoader.erase( itrParamsLoader );
				}
				else
					itrParamsLoader++;
			}
		}
/*		for( str_rendermethod& p : m_vecSubsetNameToRenderMethod[i] ) // error
		{
			p.second.m_vecpShaderParamsLoader.push_back( pShaderParamsLoader );
		}*/
	}
}


void MeshContainerRenderMethod::BreakMeshRenderMethodsToSubsetRenderMethods( const vector<string>& vecSubsetName )
{
//	size_t num_lods = m_vecMeshRenderMethod.size();

//	if( num_lods == 0 )
//		return;

//	m_vecSubsetNameToRenderMethod.resize( num_lods );
	m_vecSubsetNameToRenderMethod.resize( 1 );

//	for( size_t i=0; i<num_lods; i++ )
//	for( auto& render_method : m_vecMeshRenderMethod )
//	{
		for( const auto& subset_name : vecSubsetName )
		{
//			m_vecSubsetNameToRenderMethod[i][subset_name] = m_vecMeshRenderMethod[i];
			m_vecSubsetNameToRenderMethod[0][subset_name] = PrimaryMeshRenderMethod();
		}
//	}
}


void MeshContainerRenderMethod::RenderMeshContainer( MeshObjectContainer& mesh_container,
													 const Matrix34& world_transform )
													 //std::vector< std::shared_ptr<ShaderParamsLoader> >& vecpShaderParamsWriter )
{
	RenderMesh( mesh_container.m_MeshObjectHandle, world_transform );
}


bool MeshContainerRenderMethod::LoadRenderMethodResources()
{
	bool loaded = false;

	for( size_t i=0; i<m_RenderMethodsAndSubsetIndices.size(); i++ )
		loaded = m_RenderMethodsAndSubsetIndices[i].first.Load();

	for( size_t i=0; i<m_vecSubsetNameToRenderMethod.size(); i++ )
	{
		map< string, SubsetRenderMethod >::iterator itr;
		for( itr = m_vecSubsetNameToRenderMethod[i].begin();
			 itr != m_vecSubsetNameToRenderMethod[i].end();
			 itr++ )
		{
			loaded = (*itr).second.Load();
		}
	}

	return true;
}


/*
template<class T>
inline shared_ptr<T> create()
{
	shared_ptr<T> p( new T() );
	return p;
}
*/

shared_ptr<MeshContainerRenderMethod> MeshContainerRenderMethod::CreateCopy()
{
	shared_ptr<MeshContainerRenderMethod> pCopy( new MeshContainerRenderMethod() );

	*(pCopy.get()) = (*this);

	return pCopy;
}


void MeshContainerRenderMethod::Serialize( IArchive& ar, const unsigned int version )
{
	ar & m_vecIndicesOfSubsetsToRender;
	ar & m_LODIndex;
	ar & m_vecSubsetNameToRenderMethod;
	ar & m_RenderMethodsAndSubsetIndices;
}


void MeshContainerRenderMethod::LoadFromXMLNode( XMLNode& reader )
{
//	m_vecSubsetNameToRenderMethod.resize( 1 );

//	m_MeshRenderMethod.LoadFromXMLNode( reader );

	vector<XMLNode> vecMeshRenderMethod = reader.GetImmediateChildren( "MeshRenderMethod" );
	m_RenderMethodsAndSubsetIndices.resize( vecMeshRenderMethod.size() );
	for( size_t i=0; i<vecMeshRenderMethod.size(); i++ )
	{
		m_RenderMethodsAndSubsetIndices[i].first.LoadFromXMLNode( vecMeshRenderMethod[i] );
	}

	vector<XMLNode> vecSubsetRenderMethod = reader.GetImmediateChildren( "SubsetRenderMethod" );
	for( size_t i=0; i<vecSubsetRenderMethod.size(); i++ )
	{
		int lod = 0;
//		string lod_str = vecSubsetRenderMethod[i].GetAttributeText( "lod" );
//		if( lod_str == "" )
//			lod = 0;
//		else
//			lod = to_int(lod_str);
//
//		clamp( lod, 0, 7 );
//
//		while( (int)m_vecSubsetNameToRenderMethod.size() <= lod )
//			m_vecSubsetNameToRenderMethod.push_back( map< string, SubsetRenderMethod >() );

		string subset_name = "";
		vecSubsetRenderMethod[i].GetChildElementTextContent( "Name", subset_name );

//		shared_ptr<SubsetRenderMethod> pSubsetRenderMethod = ;
		SubsetRenderMethod subset_render_method;
		subset_render_method.LoadFromXMLNode( vecSubsetRenderMethod[i] );
		m_vecSubsetNameToRenderMethod[lod][subset_name] = subset_render_method;
	}
}



MeshContainerNodeRenderMethod::MeshContainerNodeRenderMethod()
{
}


void MeshContainerNodeRenderMethod::RenderMeshContainerNode( MeshContainerNode& node )//,
															  //std::vector< std::shared_ptr<ShaderParamsLoader> >& vecpShaderParamsWriter )
{
	const int num_mesh_containers = node.GetNumMeshContainers();
	for( int i=0; i<num_mesh_containers; i++ )
	{
		Matrix34 transform = node.GetMeshContainerWorldTransform( i );

		if( (int)m_vecpContainerRenderMethod.size() <= i )
			break;

		m_vecpContainerRenderMethod[i]->RenderMeshContainer(
			*(node.MeshContainer(i).get()),
			transform//,
			//vecpShaderParamsWriter
			);
	}

	for( int i=0; i<(int)m_vecpChild.size(); i++ )
	{
		if( node.GetNumChildren() <= (int)i )
			break;

		m_vecpChild[i]->RenderMeshContainerNode( *(node.Child( i ).get())/*, vecpShaderParamsWriter*/ );
	}
}


/// Load shaders, etc.
bool MeshContainerNodeRenderMethod::LoadRenderMethodResources()
{
	if( m_vecpContainerRenderMethod.size() == 0
	 && m_vecpChild.size() == 0 )
	{
		// nothing to load
		return false;
	}

	bool loaded = false;
	bool loaded_so_far = true;
	for( size_t i=0; i<m_vecpContainerRenderMethod.size(); i++ )
	{
		loaded = m_vecpContainerRenderMethod[i]->LoadRenderMethodResources();
		if( !loaded )
			loaded_so_far = false;
	}

	// recursively load the meshses of the child nodes
	for( size_t i=0; i<m_vecpChild.size(); i++ )
	{
		loaded = m_vecpChild[i]->LoadRenderMethodResources();
		if( !loaded )
			loaded_so_far = false;
	}

	return loaded_so_far;
}


void MeshContainerNodeRenderMethod::Serialize( IArchive& ar, const unsigned int version )
{
	ar & m_vecpContainerRenderMethod;
	ar & m_vecpChild;
}



/// Use "MeshNode" as the element name of the argument 'reader'
void MeshContainerNodeRenderMethod::LoadFromXMLNode( XMLNode& reader )
{
	vector<XMLNode> vecReader = reader.GetImmediateChildren( "MeshContainerRenderMethod" );
	m_vecpContainerRenderMethod.resize( vecReader.size() );
	for( size_t i=0; i<vecReader.size(); i++ )
	{
		m_vecpContainerRenderMethod[i].reset( new MeshContainerRenderMethod() );
		m_vecpContainerRenderMethod[i]->LoadFromXMLNode( vecReader[i] );
	}

	vector<XMLNode> vecChild = reader.GetImmediateChildren( "MeshContainerNodeRenderMethod" );
	m_vecpChild.resize( vecChild.size() );
	for( size_t i=0; i<vecChild.size(); i++ )
	{
		m_vecpChild[i].reset( new MeshContainerNodeRenderMethod() ); 
		m_vecpChild[i]->LoadFromXMLNode( vecChild[i] );
	}
}


} // namespace amorphous
