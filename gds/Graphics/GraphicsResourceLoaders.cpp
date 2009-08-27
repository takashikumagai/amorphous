#include "GraphicsResourceLoaders.hpp"
#include "GraphicsResourceCacheManager.hpp"
#include "AsyncResourceLoader.hpp"
#include "D3DXMeshObjectBase.hpp"
#include "Graphics/MeshModel/3DMeshModelArchive.hpp"
#include "Support/Profile.hpp"

using namespace std;
using namespace boost;


//===================================================================================
// CGraphicsResourceLoader
//===================================================================================

bool CGraphicsResourceLoader::Load()
{
	return LoadFromDisk();
}


bool CGraphicsResourceLoader::LoadFromDisk()
{
	bool loaded = false;
	string target_filepath;
	const string src_filepath = GetSourceFilepath();

	if( is_db_filepath_and_keyname( src_filepath ) )
	{
		// decompose the string
		string db_filename, keyname;
		decompose_into_db_filepath_and_keyname( src_filepath, db_filename, keyname );

		string cwd = fnop::get_cwd();

		//if( !boost::filesystem::exists(  ) )
		//	return InvalidPath; db path is invalid - abort

		CBinaryDatabase<string> db;
		bool db_open = db.Open( db_filename );
		if( !db_open )
			return false; // the database is being used by someone else - retry later

		loaded = LoadFromDB( db, keyname );

		target_filepath = db_filename;
	}
	else
	{
		loaded = LoadFromFile( src_filepath );

		target_filepath = src_filepath;
	}

	return loaded;
}


bool CGraphicsResourceLoader::AcquireResource()
{
	shared_ptr<CGraphicsResourceEntry> pHolder = GetResourceEntry();

	if( !pHolder )
		return false;

	shared_ptr<CGraphicsResource> pResource = GraphicsResourceCacheManager().GetCachedResource( *GetDesc() );

	if( pResource )
	{
		// set resource to entry (holder)
		pHolder->SetResource( pResource );

		return true;
	}
	else
	{
		// create resource instance from desc
		// - Actual resource creation is not done in this call
		pResource = GraphicsResourceFactory().CreateGraphicsResource( *GetDesc() );

		pHolder->SetResource( pResource );

		// desc has been set to the resource object
		//  - create the actual resource from the desc
		return pResource->Create();
	}
}


bool CGraphicsResourceLoader::Lock()
{
	LOG_FUNCTION_SCOPE();

	shared_ptr<CGraphicsResource> pResource = GetResource();
	if( pResource )
		return pResource->Lock();
	else
		return false;
}


bool CGraphicsResourceLoader::Unlock()
{
	LOG_FUNCTION_SCOPE();

	shared_ptr<CGraphicsResource> pResource = GetResource();
	if( pResource )
		return pResource->Unlock();
	else
		return false;
}


void CGraphicsResourceLoader::OnLoadingCompleted( boost::shared_ptr<CGraphicsResourceLoader> pSelf )
{
	FillResourceDesc();

	// - send a lock request
	//   to copy the loaded resource to some graphics memory
	CGraphicsDeviceRequest req( CGraphicsDeviceRequest::Lock, pSelf, GetResourceEntry() );

	AsyncResourceLoader().AddGraphicsDeviceRequest( req );
}


void CGraphicsResourceLoader::OnResourceLoadedOnGraphicsMemory()
{
	if( GetResource() )
		GetResource()->SetState( GraphicsResourceState::LOADED );
}



//===================================================================================
// CDiskTextureLoader
//===================================================================================

bool CDiskTextureLoader::LoadFromFile( const std::string& filepath )
{
	return m_Image.LoadFromFile( GetSourceFilepath() );

	//>>> async loading debug
/*	bool loaded = m_Image.LoadFromFile( GetSourceFilepath() );
	if( loaded )
	{
		// See if the image is properly loaded
		string dest_filepath = GetSourceFilepath();
		fnop::append_to_body( dest_filepath, "_loaded" );
		m_Image.SaveToFile( dest_filepath );
	}
	return loaded;*/
	//<<< async loading debug
}


bool CDiskTextureLoader::LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname )
{
	CImageArchive img_archive;
	bool retrieved = db.GetData( keyname, img_archive );
	if( retrieved )
	{
		bool image_loaded = m_Image.CreateFromImageArchive( img_archive );
		return image_loaded;
	}
	else
		return false;
}


bool CDiskTextureLoader::CopyLoadedContentToGraphicsResource()
{
	shared_ptr<CGraphicsResourceEntry> pEntry = GetResourceEntry();
	if( !pEntry )
		return false;

	shared_ptr<CTextureResource> pTexture = pEntry->GetTextureResource();

	if( !pTexture )
		return false;

	CLockedTexture locked_texture;
	bool retrieved = pTexture->GetLockedTexture( locked_texture );
	if( retrieved )
	{
		FillTexture( locked_texture );
		return true;
	}
	else
		return false;
}


void CDiskTextureLoader::FillResourceDesc()
{
	m_TextureDesc.Width  = m_Image.GetWidth();
	m_TextureDesc.Height = m_Image.GetHeight();

	m_TextureDesc.Format = TextureFormat::A8R8G8B8;
}


void CDiskTextureLoader::FillTexture( CLockedTexture& texture )
{
//	if( !m_Image.IsLoaded() )
//		return;

//	texture.Clear( SFloatRGBAColor( 1.0f, 1.0f, 1.0f, 0.0f ) );

	const int w = m_Image.GetWidth();
	const int h = m_Image.GetHeight();
	if( FreeImage_GetBPP( m_Image.GetFBITMAP() ) == 24 )
	{
		// probably an image without alpha channel
		RGBQUAD quad;
		for( int y=0; y<h; y++ )
		{
			for( int x=0; x<w; x++ )
			{
//				FreeImage_GetPixelColor( m_Image.GetFBITMAP(), x, y, &quad );
				FreeImage_GetPixelColor( m_Image.GetFBITMAP(), x, h - y - 1, &quad );
				U32 argb32
					= 0xFF000000
					| quad.rgbRed   << 16
					| quad.rgbGreen <<  8
					| quad.rgbBlue;

				texture.SetPixelARGB32( x, y, argb32 );
//				texture.SetPixelARGB32( x, y, 0xFF00FF00 ); / debug
			}
		}
	}
	else
	{
		for( int y=0; y<h; y++ )
		{
			for( int x=0; x<w; x++ )
			{
				texture.SetPixelARGB32( x, y, m_Image.GetPixelARGB32(x,y) );
			}
		}
	}
}


//===================================================================================
// CMeshLoader
//===================================================================================

CMeshLoader::CMeshLoader( boost::weak_ptr<CGraphicsResourceEntry> pEntry, const CMeshResourceDesc& desc )
:
CGraphicsResourceLoader(pEntry),
m_MeshDesc(desc),
m_MeshLoaderStateFlags(0),
m_pVertexBufferContent(NULL)
{
	m_pArchive = boost::shared_ptr<C3DMeshModelArchive>( new C3DMeshModelArchive() );
}


CMeshLoader::~CMeshLoader()
{
	SafeDelete( m_pVertexBufferContent );
}


bool CMeshLoader::LoadFromFile( const std::string& filepath )
{
	// load mesh archive from file
	bool res = false;
	if( m_pArchive )
		res = m_pArchive->LoadFromFile( GetSourceFilepath() );

	if( !res )
		return false;

	// load vertices, vertex size, vertex elements
	// from the vertex set of the mesh archive

	// load indices

	LoadMeshSubresources();

	return true;
}


bool CMeshLoader::LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname )
{
	return db.GetData( keyname, *m_pArchive );
}


bool CMeshLoader::CopyLoadedContentToGraphicsResource()
{
	// Done by sub resource loaders

	if( !GetResourceEntry() )
		return false;
/*
	CMeshResource *pMesh = GetResourceEntry()->GetMeshResource();
	if( !pMeshEntry )
		return false;

	CD3DXMeshObjectBase *pMesh = pMeshEntry->GetMeshObject();
	if( !pMesh )
		return false;
*/
	// copy vertices to VB / IB?

//	pMesh->


	return true;
}


bool CMeshLoader::AcquireResource()
{
	bool res = CGraphicsResourceLoader::AcquireResource();
	if( !res )
		return false;

	GetResourceEntry()->GetMeshResource()->CreateMeshAndLoadNonAsyncResources( *(m_pArchive.get()) );

	return true;
}


/// Called by I/O thread after the mesh archive is loaded and stored to 'm_pArchive'
/// - Usually loaded from disk
void CMeshLoader::OnLoadingCompleted( boost::shared_ptr<CGraphicsResourceLoader> pSelf )
{
	// change this to true if async loading is fixed
	bool preferred_async_loading_method = false;

	if( !preferred_async_loading_method )
	{
		CGraphicsDeviceRequest req( CGraphicsDeviceRequest::LoadToGraphicsMemoryByRenderThread, pSelf, GetResourceEntry() );
		AsyncResourceLoader().AddGraphicsDeviceRequest( req );
		return;
	}

	// - send a lock request
	//   - Actually does not lock. Send the lock request to call AcquireResource.
	//   - Must be processed before the lock requrests of the subresources below.
	CGraphicsDeviceRequest req( CGraphicsDeviceRequest::Lock, pSelf, GetResourceEntry() );
	AsyncResourceLoader().AddGraphicsDeviceRequest( req );

	// create subresource loaders
	shared_ptr<CD3DXMeshLoaderBase> apLoader[3];
	apLoader[0] = shared_ptr<CD3DXMeshVerticesLoader>( new CD3DXMeshVerticesLoader(GetResourceEntry()) );
	apLoader[1] = shared_ptr<CD3DXMeshIndicesLoader>( new CD3DXMeshIndicesLoader(GetResourceEntry()) );
	apLoader[2] = shared_ptr<CD3DXMeshAttributeTableLoader>( new CD3DXMeshAttributeTableLoader(GetResourceEntry()) );

	for( int i=0; i<3; i++ )
	{
		// set the pointer to mesh archive to each subresource loader
		apLoader[i]->m_pArchive   = m_pArchive;
//		apLoader[i]->m_pMeshEntry = m_pMeshEntry;

		apLoader[i]->m_pMeshLoader = m_pSelf.lock();

		// add requests to load subresource from the mesh archive
//		CResourceLoadRequest req( CResourceLoadRequest::LoadFromDisk, apLoader[i], GetResourceEntry() );
//		AsyncResourceLoader().AddResourceLoadRequest( req );

		// subresources have been loaded
		// - send lock requests for each mesh sub resource
		CGraphicsDeviceRequest req( CGraphicsDeviceRequest::Lock, apLoader[i], GetResourceEntry() );
		AsyncResourceLoader().AddGraphicsDeviceRequest( req );
	}

	// create mesh instance
	// load resources that needs to be loaded synchronously
//	GetResourceEntry()->GetMeshResource()->CreateMeshAndLoadNonAsyncResources( *(m_pArchive.get()) );
}


void CMeshLoader::OnResourceLoadedOnGraphicsMemory()
{
	// Do nothing
	// - Avoid setting the resource state to GraphicsResourceState::LOADED
	// - Need to wait until all the subresources are locked, copied, and unlocked.
}


void CMeshLoader::LoadMeshSubresources()
{
	if( m_pArchive )
	{
		m_MeshDesc.NumVertices = m_pArchive->GetVertexSet().GetNumVertices();
		m_MeshDesc.NumIndices  = (int)m_pArchive->GetNumVertexIndices();

		m_MeshDesc.VertexFormatFlags = m_pArchive->GetVertexSet().m_VertexFormatFlag;

		LoadVerticesForD3DXMesh(
			m_pArchive->GetVertexSet(),
			m_MeshDesc.vecVertElement,
			m_MeshDesc.VertexSize,
			m_pVertexBufferContent
			);

//		SafeDelete( pVertexBufferContent );

		LoadIndices( *(m_pArchive.get()), m_vecIndexBufferContent );

		GetAttributeTableFromTriangleSet( m_pArchive->GetTriangleSet(), m_vecAttributeRange );
	}
}


void CMeshLoader::RaiseStateFlags( U32 flags )
{
	m_MeshLoaderStateFlags |= flags;
}


void CMeshLoader::SendLockRequestIfAllSubresourcesHaveBeenLoaded()
{
	if( m_MeshLoaderStateFlags & VERTICES_LOADED
	 && m_MeshLoaderStateFlags & INDICES_LOADED
	 && m_MeshLoaderStateFlags & ATTRIB_TABLES_LOADED )
	{
		return;
	}
	else
	{
		return;
	}
}


bool CMeshLoader::LoadToGraphicsMemoryByRenderThread()
{
	if( !m_pArchive )
		return false;

	// Make the system create an empty mesh instance by settings vertices and indices to zeros 
	m_MeshDesc.NumVertices = 0;
	m_MeshDesc.NumIndices = 0;

	bool res = CGraphicsResourceLoader::AcquireResource();
	if( !res )
		return false;

	shared_ptr<CGraphicsResourceEntry> pHolder = GetResourceEntry();
	if( !pHolder )
		return false;

	shared_ptr<CMeshResource> pMeshResource = pHolder->GetMeshResource();
	if( !pMeshResource )
		return false;

	shared_ptr<CD3DXMeshObjectBase> pMesh = pMeshResource->GetMesh();
	if( pMesh )
		return pMesh->LoadFromArchive( *(m_pArchive.get()), m_MeshDesc.ResourcePath );
	else
		return false;
}



//===================================================================================
// CD3DXMeshVerticesLoader
//===================================================================================

bool CD3DXMeshVerticesLoader::LoadFromArchive()
{
/*	// load the vertices from the mesh archive (m_pArchive), and store it
	// to the buffer (m_pVertexBufferContent).
	GetMesh()->LoadVertices( m_pVertexBufferContent, *(m_pArchive.get()) );

//	LoadVerticesForD3DXMesh( m_pArchive->GetVertexSet(), elems, size, dest_buffer );

	m_pMeshLoader->RaiseStateFlags( CMeshLoader::VERTICES_LOADED );
	m_pMeshLoader->SendLockRequestIfAllSubresourcesHaveBeenLoaded();
*/
	return true;
}


bool CD3DXMeshVerticesLoader::CopyLoadedContentToGraphicsResource()
{
	LOG_PRINT( "" );

	if( m_pLockedVertexBuffer )
	{
//		memcpy( m_pLockedVertexBuffer, m_pVertexBufferContent, GetMesh()->GetVertexSize() * m_pArchive->GetVertexSet().GetNumVertices() );

		memcpy( m_pLockedVertexBuffer,
			m_pMeshLoader->VertexBufferContent(),
			GetMesh()->GetVertexSize() * m_pArchive->GetVertexSet().GetNumVertices() );
	}

	return true;
}


bool CD3DXMeshVerticesLoader::Lock()
{
	LOG_PRINT( "" );

	CD3DXMeshObjectBase *pMesh = GetMesh();
	if( pMesh )
		return pMesh->LockVertexBuffer( m_pLockedVertexBuffer );
	else
		return false;
}


bool CD3DXMeshVerticesLoader::Unlock()
{
	LOG_PRINT( "" );

	bool unlocked = GetMesh()->UnlockVertexBuffer();
	m_pLockedVertexBuffer = NULL;
	return unlocked;
}


void CD3DXMeshVerticesLoader::OnResourceLoadedOnGraphicsMemory()
{
	SetSubResourceState( CMeshSubResource::VERTEX, GraphicsResourceState::LOADED );
}


bool CD3DXMeshVerticesLoader::IsReadyToLock() const
{
	return true;
}



//===================================================================================
// CD3DXMeshIndicesLoader
//===================================================================================

bool CD3DXMeshIndicesLoader::LoadFromArchive()
{
/*	unsigned short *pIBData;
	GetMesh()->LoadIndices( pIBData, *(m_pArchive.get()) );
	m_pIndexBufferContent = (void *)pIBData;

	m_pMeshLoader->RaiseStateFlags( CMeshLoader::INDICES_LOADED );
	m_pMeshLoader->SendLockRequestIfAllSubresourcesHaveBeenLoaded();
*/
	return true;
}


bool CD3DXMeshIndicesLoader::Lock()
{
	LOG_PRINT( "" );

	CD3DXMeshObjectBase *pMesh = GetMesh();
	if( pMesh )
		return pMesh->LockIndexBuffer( m_pLockedIndexBuffer );
	else
		return false;
}


bool CD3DXMeshIndicesLoader::Unlock()
{
	LOG_PRINT( "" );

	bool unlocked = GetMesh()->UnlockIndexBuffer();
	m_pLockedIndexBuffer = NULL;
	return unlocked;
}


bool CD3DXMeshIndicesLoader::CopyLoadedContentToGraphicsResource()
{
	LOG_PRINT( "" );

	if( m_pLockedIndexBuffer )
	{
//		memcpy( m_pLockedIndexBuffer, m_pIndexBufferContent, m_IndexBufferSize );

		memcpy( m_pLockedIndexBuffer,
			&(m_pMeshLoader->IndexBufferContent()[0]),
			m_pMeshLoader->IndexBufferContent().size() );
	}


	return true;
}


void CD3DXMeshIndicesLoader::OnResourceLoadedOnGraphicsMemory()
{
	SetSubResourceState( CMeshSubResource::INDEX, GraphicsResourceState::LOADED );
}


bool CD3DXMeshIndicesLoader::IsReadyToLock() const
{
	return GetSubResourceState( CMeshSubResource::VERTEX ) == GraphicsResourceState::LOADED;
}



//===================================================================================
// CD3DXMeshAttributeTableLoader
//===================================================================================

bool CD3DXMeshAttributeTableLoader::Lock()
{
	LOG_PRINT( "" );

	HRESULT hr = GetMesh()->GetMesh()->SetAttributeTable(
		&(m_pMeshLoader->AttributeTable()[0]),
		(DWORD)(m_pMeshLoader->AttributeTable().size()) );

	bool locked = GetMesh()->LockAttributeBuffer( m_pLockedAttributeBuffer );
	return locked;
}


bool CD3DXMeshAttributeTableLoader::Unlock()
{
	LOG_PRINT( "" );

	bool unlocked = GetMesh()->UnlockAttributeBuffer();
	m_pLockedAttributeBuffer = NULL;
	return unlocked;
}


bool CD3DXMeshAttributeTableLoader::CopyLoadedContentToGraphicsResource()
{
	LOG_PRINT( "" );

	if( !m_pLockedAttributeBuffer )
		return false;

	const vector<CMMA_TriangleSet>& vecTriangleSet = m_pArchive->GetTriangleSet();

	DWORD *pdwBuffer = m_pLockedAttributeBuffer;
	DWORD face = 0;
	const int num_materials = GetMesh()->GetNumMaterials();
	for( int i=0; i<num_materials; i++ )
	{
		const CMMA_TriangleSet& triangle_set = vecTriangleSet[i];

		DWORD face_start = triangle_set.m_iStartIndex / 3;
		DWORD num_faces = triangle_set.m_iNumTriangles;
		for( face=face_start; face<face_start + num_faces; face++ )
		{
			pdwBuffer[face] = i;
		}
	}

	return true;
}


void CD3DXMeshAttributeTableLoader::OnResourceLoadedOnGraphicsMemory()
{
	SetSubResourceState( CMeshSubResource::ATTRIBUTE_TABLE, GraphicsResourceState::LOADED );
}


bool CD3DXMeshAttributeTableLoader::IsReadyToLock() const
{
	return GetSubResourceState( CMeshSubResource::INDEX ) == GraphicsResourceState::LOADED;
}



/// Cannot create an empty shader object and copy the content to it.
/// - Load from file
bool CShaderLoader::AcquireResource()
{
	shared_ptr<CGraphicsResourceEntry> pHolder = GetResourceEntry();

	if( !pHolder )
		return false;

	pHolder->SetResource( GraphicsResourceFactory().CreateGraphicsResource( m_ShaderDesc ) );

	shared_ptr<CShaderResource> pShaderResource = pHolder->GetShaderResource();
	if( !pShaderResource )
		return false;

	bool loaded = pShaderResource->LoadFromFile( m_ShaderDesc.ResourcePath );

	return loaded;
}
