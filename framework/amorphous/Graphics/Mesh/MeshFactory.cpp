#include "MeshFactory.hpp"
#include "ProgressiveMesh.hpp"
#include "SkeletalMesh.hpp"
#include "CustomMesh.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"


namespace amorphous
{

using namespace std;


void SetCustomMesh( BasicMesh& src_mesh )
{
	src_mesh.m_pImpl.reset( new CustomMesh );
}


//=============================================================================
// MeshFactory
//=============================================================================

BasicMesh *MeshFactory::InitMeshInstance( MeshTypeName mesh_type, U32 load_option_flags )
{
	BasicMesh* pMesh = CreateMeshInstance( mesh_type );
	if( !pMesh )
		return nullptr;

	if( load_option_flags & MeshLoadOption::CUSTOM_MESH )
		SetCustomMesh( *pMesh );

	return pMesh;
}


BasicMesh *MeshFactory::CreateMeshInstance( MeshTypeName mesh_type )
{
	switch( mesh_type )
	{
	case MeshTypeName::BASIC:
		return CreateBasicMeshInstance();
	case MeshTypeName::PROGRESSIVE:
		return CreateProgressiveMeshInstance();
	case MeshTypeName::SKELETAL:
		return CreateSkeletalMeshInstance();
	default:
		return nullptr;
	}

	return nullptr;
}


shared_ptr<BasicMesh> MeshFactory::CreateMesh( MeshTypeName mesh_type )
{
	return shared_ptr<BasicMesh>( CreateMeshInstance( mesh_type ) );
}


BasicMesh *MeshFactory::CreateBasicMeshInstance() { return new BasicMesh; }
ProgressiveMesh *MeshFactory::CreateProgressiveMeshInstance() { return new ProgressiveMesh; } 
SkeletalMesh *MeshFactory::CreateSkeletalMeshInstance() { return new SkeletalMesh; } 

shared_ptr<BasicMesh> MeshFactory::CreateBasicMesh() { shared_ptr<BasicMesh> pMesh( CreateBasicMeshInstance() ); return pMesh; }
shared_ptr<ProgressiveMesh> MeshFactory::CreateProgressiveMesh() { shared_ptr<ProgressiveMesh> pMesh( CreateProgressiveMeshInstance() ); return pMesh; } 
shared_ptr<SkeletalMesh> MeshFactory::CreateSkeletalMesh() { shared_ptr<SkeletalMesh> pMesh( CreateSkeletalMeshInstance() ); return pMesh; } 


BasicMesh* MeshFactory::LoadMeshObjectFromFile( const std::string& filepath,
												  U32 load_option_flags,
												  MeshTypeName mesh_type )
{
	BasicMesh* pMesh = InitMeshInstance( mesh_type, load_option_flags );

	bool loaded = pMesh->LoadFromFile( filepath, load_option_flags );

	if( loaded )
		return pMesh;
	else
	{
		SafeDelete( pMesh );
		return nullptr;
	}
}


BasicMesh* MeshFactory::LoadMeshObjectFromArchive( C3DMeshModelArchive& mesh_archive,
																    const std::string& filepath,
																    U32 load_option_flags,
																	MeshTypeName mesh_type )
{
	BasicMesh* pMesh = InitMeshInstance( mesh_type, load_option_flags );

	bool loaded = pMesh->LoadFromArchive( mesh_archive, filepath, load_option_flags );

	if( loaded )
		return pMesh;
	else
	{
		SafeDelete( pMesh );
		return nullptr;
	}
}


BasicMesh* MeshFactory::LoadMeshFromMemory(
	const void *buffer,
	int buffer_size_in_bytes,
	const std::string& filepath,
	U32 load_option_flags,
	MeshTypeName mesh_type
)
{
	C3DMeshModelArchive ar;
	LOG_PRINTF(("data size in bytes: %d",buffer_size_in_bytes));
	bool loaded = ar.LoadFromMemory(buffer,buffer_size_in_bytes);

	if(!loaded)
	{
		return nullptr;
	}

	return LoadMeshObjectFromArchive( ar, filepath, load_option_flags, mesh_type );
}

} // namespace amorphous
