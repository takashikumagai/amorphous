#include "AsyncResourceLoader.h"
#include <boost/thread/xtime.hpp>

using namespace std;
using namespace boost;


class Result
{
public:
	enum Name
	{
		SUCCESS = 0,
		RESOURCE_NOT_FOUND,
		RESOURCE_IN_USE,
		NUM_RESULTS
	};
};


CSingleton<CAsyncResourceLoader> CAsyncResourceLoader::m_obj;


CAsyncResourceLoader::CAsyncResourceLoader()
:
m_bEndIOThread(false)
{
	if( !GraphicsResourceManager().IsAsyncLoadingAllowed() )
	{
		// asynchronous loading is turned off
		return;
	}

	// create resource IO thread, which
	// - loads resources (textures, meshes, etc.) from disk to memory
	// - copy resources loaded on memory to locked graphics resource memory
	m_pIOThread = shared_ptr<thread>( new thread( CIOThreadStarter(this) ) );
}


/// Release the resource IO thread
void CAsyncResourceLoader::Release()
{
	if( m_pIOThread )
	{
		m_bEndIOThread = true;
		m_pIOThread->join();
		m_pIOThread.reset();
	}
}


bool CAsyncResourceLoader::AddResourceLoadRequest( const CResourceLoadRequest& req )
{
	mutex::scoped_lock scoped_lock(m_IOMutex);

	m_ResourceLoadRequestQueue.push( req );

	return true;
}


void CAsyncResourceLoader::IOThreadMain()
{
	ProcessResourceLoadRequests();
}


/// called by file IO thread
void CAsyncResourceLoader::ProcessResourceLoadRequests()
{
	bool copied = false;
	bool res = false;
//	Result::Name res;
	shared_ptr<CGraphicsResourceLoader> pLoader;
	CResourceLoadRequest req( CResourceLoadRequest::LoadFromDisk, weak_ptr<CGraphicsResourceEntry>() );

	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec += 1; // 1 [sec]

	while( !m_bEndIOThread )
	{
		boost::thread::sleep(xt);

		if( 0 < m_ResourceLoadRequestQueue.size() )
		{
			mutex::scoped_lock scoped_lock(m_IOMutex);
			req = m_ResourceLoadRequestQueue.front();
			m_ResourceLoadRequestQueue.pop();
		}

		switch( req.GetRequestType() )
		{
		case CResourceLoadRequest::LoadFromDisk:
			pLoader = req.m_pLoader;
			// load the resource from the disk
/*			res = pLoader->LoadFromDisk();
			switch(res)
			{
			case LoadResult::SUCCESS:
				break;
			case LoadResult::RESOURCE_NOT_FOUND:
				continue;
				break;
			case LoadResult::RESOURCE_IN_USE:
				m_ResourceLoadRequestQueue.push( req );
				continue;
				// try again later
				break;
			default:
				break;
			}*/

			res = pLoader->LoadFromDisk();

			if( res )
			{
				// loaded the resource
				// - send lock request to render thread in order to
				//   copy the loaded resource to some graphics memory
				mutex::scoped_lock scoped_lock(m_GraphicsDeviceMutex);
				m_GraphicsDeviceRequestQueue.push(
					CGraphicsDeviceRequest( CGraphicsDeviceRequest::Lock, req.m_pResourceEntry )
					);
			}
			break;

		case CResourceLoadRequest::CopyToGraphicsMemory:
			// copy loaded data to locked graphics memory
			copied = req.m_pLoader->CopyTo( req.m_pResourceEntry.lock().get() );
			if( copied )
			{
				mutex::scoped_lock scoped_lock(m_GraphicsDeviceMutex);
				m_GraphicsDeviceRequestQueue.push(
					CGraphicsDeviceRequest( CGraphicsDeviceRequest::Unlock, req.m_pResourceEntry.lock() )
					);
			}
			
			break;

		default:
			break;
		}
	}
}


void CAsyncResourceLoader::ProcessGraphicsDeviceRequests()
{
	bool resource_locked = false;
	shared_ptr<CGraphicsResourceEntry> pSrcEntry, pEntry;
	CGraphicsDeviceRequest req( CGraphicsDeviceRequest::Lock, weak_ptr<CGraphicsResourceEntry>() );

	for( int i=0; i<1; i++ )
	{
		{
			mutex::scoped_lock scoped_lock(m_GraphicsDeviceMutex);
			req = m_GraphicsDeviceRequestQueue.front();
			m_GraphicsDeviceRequestQueue.pop();
		}

		switch( req.m_RequestType )
		{
		case CGraphicsDeviceRequest::Lock:
			{
				// find a resource entry that matches the resource description
				pSrcEntry = req.m_pResourceEntry.lock();
				if( !pSrcEntry )
					continue;

				// texture / mesh sizes are stored in the loader
				// - GraphicsResourceManager can determine the graphics resource entry that matches the specification.
				//   It either,
				//   - Create a new graphics resource entry.
				//   - Draw one from the cache.
				// - This will overwrite the slot that has been occupied by pSrcEntry. i.e., ref count of pSrcEntry
				//   will be zero after leaving this scope
				pEntry = GraphicsResourceManager().CreateAt( pSrcEntry->GetDesc(), pSrcEntry->GetIndex() );
				if( pEntry )
				{
					// update the entry
					req.m_pResourceEntry = pEntry;

					// refresh the entry held by the loader
					// - Should separate resource entry class and resource class to avoid this?
					req.m_pLoader->SetEntry( pEntry );

//					resource_locked = pEntry->Lock(); // Does not work for the mesh, since it has 3 buffers to lock - VB, IB, and attribute buffer

					req.m_pLoader->Lock();
				}

				// mark the entry as locked

				// graphics resource memory has been locked and it's ready to receive data loaded from the disk
				// - add a copy request to IO queue
				CResourceLoadRequest copy_req( CResourceLoadRequest::CopyToGraphicsMemory, pEntry );
				m_ResourceLoadRequestQueue.push( copy_req );

				// save the weak/shared ptr to the entry
//				desc/req.pLockedEntry = ;
			}
			break;

		case CGraphicsDeviceRequest::Unlock:
			{
				pEntry = req.m_pResourceEntry.lock();
				if( pEntry )
					pEntry->Unlock();

				pEntry->SetState( GraphicsResourceState::LOADED );
			}
			break;
		}
	}
}
