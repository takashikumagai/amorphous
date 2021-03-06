#ifndef  __GraphicsResourceLoaders_H__
#define  __GraphicsResourceLoaders_H__


#include "GraphicsResources.hpp"
#include "GraphicsResourceEntries.hpp"
#include "../Support/SafeDelete.hpp"
#include "../Support/stream_buffer.hpp"
#include "../Support/Serialization/BinaryDatabase.hpp"


namespace amorphous
{
using namespace serialization;


class BitmapImage;


const std::string g_NullString = "";


class GraphicsResourceLoader
{
	/// entry that stores the loaded resource
	std::weak_ptr<GraphicsResourceEntry> m_pResourceEntry;

protected:

//	virtual std::shared_ptr<GraphicsResourceEntry> GetResourceEntry() = 0;

	std::shared_ptr<GraphicsResourceEntry> GetResourceEntry() { return m_pResourceEntry.lock(); }

	const std::shared_ptr<GraphicsResourceEntry> GetResourceEntry() const { return m_pResourceEntry.lock(); }

	inline const std::string& GetSourceFilepath();

public:

	GraphicsResourceLoader( std::weak_ptr<GraphicsResourceEntry> pEntry )
		:
	m_pResourceEntry(pEntry)
	{}

	inline std::shared_ptr<GraphicsResource> GetResource();

	/// Loads resource, e.g. from a file, to some generic heap memory
	/// Note that this function must not call APIs of graphics library
	virtual Result::Name Load();

	Result::Name LoadFromDisk();

	virtual bool LoadFromFile( const std::string& filepath ) { return false; }

	virtual bool LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname ) { return false; }

	/// Called by the resource IO thread
	/// - copy the loaded resource to locked buffer
	virtual bool CopyLoadedContentToGraphicsResource() { return false; }

	virtual bool AcquireResource();

	/// Called by the rendering thread
	/// - Why virtual ?
	///   -> Mesh resources need separate locks and unlocks for subresources
	///   -> Texture resources need separate locks for different mipmap levels
	virtual bool Lock();

	/// Called by the rendering thread
	/// - Why virtual ?
	///   -> See Lock() above
	virtual bool Unlock();

//	virtual void SetEntry( std::weak_ptr<GraphicsResourceEntry> pEntry ) = 0;

	/// add a lock request to the graphics device request queue of the async resource loader
	virtual void OnLoadingCompleted( std::shared_ptr<GraphicsResourceLoader> pSelf );

	/// - Called by async resource loader when the resource is loaded on locked graphics memory
	///   and the memory is successfully unlocked.
	/// - By default, this method set the resource state to GraphicsResourceState::LOADED.
	virtual void OnResourceLoadedOnGraphicsMemory();

	virtual bool IsReadyToLock() const { return true; }

	/// Used to fill out desc properties that can be obtained
	/// only after the resource is loaded from disk
	/// - e.g., width and height of image files for textures
	virtual void FillResourceDesc() {}

	/// sub resource loaders of mesh don't have descs
	virtual const GraphicsResourceDesc *GetDesc() const { return NULL; }

	virtual bool LoadToGraphicsMemoryByRenderThread() { return false; }

	/// \brief Loads everything using render thread
	///
	/// Use this until async loading system is re-designed,
	/// Note that this not free the render thread from loading tasks,
	/// such as creating mipmaps.
	bool LoadResourceAndCreateGraphicsResource();
};


/// loads a texture from disk
class DiskTextureLoader : public GraphicsResourceLoader
{
	/// Stores image properties such as width and height.
	/// - Image properties are obtained from m_Image after the image is loaded
	TextureResourceDesc m_TextureDesc;

	/// Stores texture data loaded from disk
//	std::shared_ptr<BitmapImage> m_pImage;
	std::vector< std::shared_ptr<BitmapImage> > m_vecpImage;

	std::weak_ptr<DiskTextureLoader> m_pSelf;

	int m_CurrentMipLevel;

	std::shared_ptr<TextureResource> GetTextureResource();

protected:

//	std::shared_ptr<GraphicsResourceEntry> GetResourceEntry() { return m_pTextureEntry.lock(); }

	/// Returns true on success
	bool InitImageArray( std::shared_ptr<BitmapImage> pBaseImage );

	/// Creates rescaled images for mipmaps.
	/// Returns true on success.
	bool CreateScaledImagesForMipmaps();

public:

	DiskTextureLoader( std::weak_ptr<GraphicsResourceEntry> pEntry, const TextureResourceDesc& desc )
		:
	GraphicsResourceLoader(pEntry),
	m_TextureDesc(desc),
	m_CurrentMipLevel(0)
	{}

	virtual ~DiskTextureLoader() {}

	// On success this creates an array of bitmap images containing the mipmaps of the specified texture image.
	bool LoadFromFile( const std::string& filepath );

	/// load image from the db as an image archive
	bool LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname );

	/// copy the bitmap image to the locked texture surface
	bool CopyLoadedContentToGraphicsResource();

	bool Lock();

	void FillResourceDesc();

	const GraphicsResourceDesc *GetDesc() const { return &m_TextureDesc; }

	/// called by the system
	/// - called inside CopyTo()
	void FillTexture( LockedTexture& texture );

	void OnResourceLoadedOnGraphicsMemory();

	void SetWeakPtr( std::weak_ptr<DiskTextureLoader> pSelf ) { m_pSelf = pSelf; }
};


class MeshLoader : public GraphicsResourceLoader
{
protected:

	MeshResourceDesc m_MeshDesc;

	std::shared_ptr<C3DMeshModelArchive> m_pArchive;

	U32 m_MeshLoaderStateFlags;

	std::weak_ptr<MeshLoader> m_pSelf;

public:

	MeshLoader( std::weak_ptr<GraphicsResourceEntry> pEntry, const MeshResourceDesc& desc );

	virtual ~MeshLoader();

	bool LoadFromFile( const std::string& filepath );

	bool LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname );

	bool CopyLoadedContentToGraphicsResource();

	bool AcquireResource();

	/// Does not lock the mesh resource
	/// - Loaders for each component of the mesh (vertices, indices, and attribute buffers)
	///   do the lock & unlock, and copy each resource to the mesh
	bool Lock() { return false; }

	/// Does not unlock the mesh resource
	/// - See the comment of Lock()
	bool Unlock() { return false; }

	virtual void OnLoadingCompleted( std::shared_ptr<GraphicsResourceLoader> pSelf ) {}

	virtual void OnResourceLoadedOnGraphicsMemory() {}

	virtual void LoadMeshSubresources() {}

	void SetWeakPtr( std::weak_ptr<MeshLoader> pSelf ) { m_pSelf = pSelf; }

	const GraphicsResourceDesc *GetDesc() const { return &m_MeshDesc; }

	void RaiseStateFlags( U32 flags );

	U32 GetMeshLoaderStateFlags() const { return m_MeshLoaderStateFlags; }

	void SendLockRequestIfAllSubresourcesHaveBeenLoaded();

	bool LoadToGraphicsMemoryByRenderThread();

	enum SubResouceLoadingStateFlag
	{
		VERTICES_LOADED      = ( 1 << 0 ),
		INDICES_LOADED       = ( 1 << 1 ),
		ATTRIB_TABLES_LOADED = ( 1 << 2 ),
	};
};


//--------------------------- inline implementations ---------------------------

//==============================================================================
// GraphicsResourceLoader
//==============================================================================

inline std::shared_ptr<GraphicsResource> GraphicsResourceLoader::GetResource()
{
	std::shared_ptr<GraphicsResourceEntry> pEntry = GetResourceEntry();

	if( pEntry )
		return pEntry->GetResource();
	else
		return std::shared_ptr<GraphicsResource>();
}


inline const std::string& GraphicsResourceLoader::GetSourceFilepath()
{
	const GraphicsResourceDesc *pDesc = GetDesc();

	if( pDesc )
	{
		return pDesc->ResourcePath;
	}
	else
		return g_NullString;

/*	std::shared_ptr<GraphicsResourceEntry> pEntry = GetResourceEntry();
	if( pEntry && pEntry->GetResource() )
		return pEntry->GetResource()->GetDesc().ResourcePath;
	else
		return g_NullString;*/
}



/// loads a shader from disk
class ShaderLoader : public GraphicsResourceLoader
{
	/// Stores filepath
	ShaderResourceDesc m_ShaderDesc;

	/// Stores shader contents
	stream_buffer m_ShaderTextBuffer;

public:

	ShaderLoader( std::weak_ptr<GraphicsResourceEntry> pEntry, const ShaderResourceDesc& desc )
		:
	GraphicsResourceLoader(pEntry),
	m_ShaderDesc(desc)
	{}

	bool LoadFromFile( const std::string& filepath );

	/// load image from the db as an image archive
//	bool LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname );

	/// copy the bitmap image to the locked texture surface
//	bool CopyLoadedContentToGraphicsResource();

//	void FillResourceDesc();

	const GraphicsResourceDesc *GetDesc() const { return &m_ShaderDesc; }

	bool CopyLoadedContentToGraphicsResource() { return true; }

	bool AcquireResource();

	bool Lock() { return true; }

	bool Unlock() { return true; }

	void OnLoadingCompleted( std::shared_ptr<GraphicsResourceLoader> pSelf );

	bool LoadToGraphicsMemoryByRenderThread();
};

} // namespace amorphous



#endif  /* __GraphicsResourceLoaders_H__ */
