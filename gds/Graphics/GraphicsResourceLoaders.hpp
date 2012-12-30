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


class CBitmapImage;


const std::string g_NullString = "";


class CGraphicsResourceLoader
{
	/// entry that stores the loaded resource
	boost::weak_ptr<CGraphicsResourceEntry> m_pResourceEntry;

protected:

//	virtual boost::shared_ptr<CGraphicsResourceEntry> GetResourceEntry() = 0;

	boost::shared_ptr<CGraphicsResourceEntry> GetResourceEntry() { return m_pResourceEntry.lock(); }

	const boost::shared_ptr<CGraphicsResourceEntry> GetResourceEntry() const { return m_pResourceEntry.lock(); }

	inline const std::string& GetSourceFilepath();

public:

	CGraphicsResourceLoader( boost::weak_ptr<CGraphicsResourceEntry> pEntry )
		:
	m_pResourceEntry(pEntry)
	{}

	inline boost::shared_ptr<CGraphicsResource> GetResource();

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

//	virtual void SetEntry( boost::weak_ptr<CGraphicsResourceEntry> pEntry ) = 0;

	/// add a lock request to the graphics device request queue of the async resource loader
	virtual void OnLoadingCompleted( boost::shared_ptr<CGraphicsResourceLoader> pSelf );

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
	virtual const CGraphicsResourceDesc *GetDesc() const { return NULL; }

	virtual bool LoadToGraphicsMemoryByRenderThread() { return false; }
};


/// loads a texture from disk
class CDiskTextureLoader : public CGraphicsResourceLoader
{
	/// Stores image properties such as width and height.
	/// - Image properties are obtained from m_Image after the image is loaded
	CTextureResourceDesc m_TextureDesc;

	/// Stores texture data loaded from disk
//	boost::shared_ptr<CBitmapImage> m_pImage;
	std::vector< boost::shared_ptr<CBitmapImage> > m_vecpImage;

	boost::weak_ptr<CDiskTextureLoader> m_pSelf;

	int m_CurrentMipLevel;

	boost::shared_ptr<CTextureResource> GetTextureResource();

protected:

//	boost::shared_ptr<CGraphicsResourceEntry> GetResourceEntry() { return m_pTextureEntry.lock(); }

	/// Returns true on success
	bool InitImageArray( boost::shared_ptr<CBitmapImage> pBaseImage );

	/// Creates rescaled images for mipmaps.
	/// Returns true on success.
	bool CreateScaledImagesForMipmaps();

public:

	CDiskTextureLoader( boost::weak_ptr<CGraphicsResourceEntry> pEntry, const CTextureResourceDesc& desc )
		:
	CGraphicsResourceLoader(pEntry),
	m_TextureDesc(desc),
	m_CurrentMipLevel(0)
	{}

	virtual ~CDiskTextureLoader() {}

	bool LoadFromFile( const std::string& filepath );

	/// load image from the db as an image archive
	bool LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname );

	/// copy the bitmap image to the locked texture surface
	bool CopyLoadedContentToGraphicsResource();

	bool Lock();

	void FillResourceDesc();

	const CGraphicsResourceDesc *GetDesc() const { return &m_TextureDesc; }

	/// called by the system
	/// - called inside CopyTo()
	void FillTexture( CLockedTexture& texture );

	void OnResourceLoadedOnGraphicsMemory();

	void SetWeakPtr( boost::weak_ptr<CDiskTextureLoader> pSelf ) { m_pSelf = pSelf; }
};


class CMeshLoader : public CGraphicsResourceLoader
{
protected:

	CMeshResourceDesc m_MeshDesc;

	boost::shared_ptr<C3DMeshModelArchive> m_pArchive;

	U32 m_MeshLoaderStateFlags;

	boost::weak_ptr<CMeshLoader> m_pSelf;

public:

	CMeshLoader( boost::weak_ptr<CGraphicsResourceEntry> pEntry, const CMeshResourceDesc& desc );

	virtual ~CMeshLoader();

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

	virtual void OnLoadingCompleted( boost::shared_ptr<CGraphicsResourceLoader> pSelf ) {}

	virtual void OnResourceLoadedOnGraphicsMemory() {}

	virtual void LoadMeshSubresources() {}

	void SetWeakPtr( boost::weak_ptr<CMeshLoader> pSelf ) { m_pSelf = pSelf; }

	const CGraphicsResourceDesc *GetDesc() const { return &m_MeshDesc; }

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
// CGraphicsResourceLoader
//==============================================================================

inline boost::shared_ptr<CGraphicsResource> CGraphicsResourceLoader::GetResource()
{
	boost::shared_ptr<CGraphicsResourceEntry> pEntry = GetResourceEntry();

	if( pEntry )
		return pEntry->GetResource();
	else
		return boost::shared_ptr<CGraphicsResource>();
}


inline const std::string& CGraphicsResourceLoader::GetSourceFilepath()
{
	const CGraphicsResourceDesc *pDesc = GetDesc();

	if( pDesc )
	{
		return pDesc->ResourcePath;
	}
	else
		return g_NullString;

/*	boost::shared_ptr<CGraphicsResourceEntry> pEntry = GetResourceEntry();
	if( pEntry && pEntry->GetResource() )
		return pEntry->GetResource()->GetDesc().ResourcePath;
	else
		return g_NullString;*/
}



/// loads a shader from disk
class CShaderLoader : public CGraphicsResourceLoader
{
	/// Stores filepath
	CShaderResourceDesc m_ShaderDesc;

	/// Stores shader contents
	stream_buffer m_ShaderTextBuffer;

public:

	CShaderLoader( boost::weak_ptr<CGraphicsResourceEntry> pEntry, const CShaderResourceDesc& desc )
		:
	CGraphicsResourceLoader(pEntry),
	m_ShaderDesc(desc)
	{}

	bool LoadFromFile( const std::string& filepath );

	/// load image from the db as an image archive
//	bool LoadFromDB( CBinaryDatabase<std::string>& db, const std::string& keyname );

	/// copy the bitmap image to the locked texture surface
//	bool CopyLoadedContentToGraphicsResource();

//	void FillResourceDesc();

	const CGraphicsResourceDesc *GetDesc() const { return &m_ShaderDesc; }

	bool CopyLoadedContentToGraphicsResource() { return true; }

	bool AcquireResource();

	bool Lock() { return true; }

	bool Unlock() { return true; }

	void OnLoadingCompleted( boost::shared_ptr<CGraphicsResourceLoader> pSelf );

	bool LoadToGraphicsMemoryByRenderThread();
};

} // namespace amorphous



#endif  /* __GraphicsResourceLoaders_H__ */
