#include "ConvexPolygon.hpp"
#include "amorphous/Support/memory_helpers.hpp"


namespace amorphous
{


ConvexPolygon::ConvexPolygon()
{
	m_iNumVertices = 0;
	m_pavVertex = nullptr;

	m_iPolygonIndex = -1;
	m_uiCollisionFlag = 0;
}


ConvexPolygon::ConvexPolygon( Vector3 *pavVertex, int iNumVertices )
{
	m_pavVertex = nullptr;
	SetVertices( pavVertex, iNumVertices );
}


ConvexPolygon::~ConvexPolygon()
{
	SafeDeleteArray( m_pavVertex );
}


void ConvexPolygon::SetVertices( Vector3 *pavVertex, int iNumVertices )
{
	SafeDeleteArray( m_pavVertex );

	m_iNumVertices = iNumVertices;
	m_pavVertex = new Vector3 [iNumVertices];
	int i;
	for(i=0; i<iNumVertices; i++)
	{
		m_pavVertex[i] = pavVertex[i];
//		D3DXVECTOR3& v = pavVertex[i];
///		m_pavVertex[i].setValue( (float *)&pavVertex[i] );
	}

	ComputeAABB();
}


void ConvexPolygon::ComputeAABB()
{
	Vector3 vMin = Vector3( 99999, 99999, 99999);
	Vector3	vMax = Vector3(-99999,-99999,-99999);

	int i, iAxis;
	for(i=0; i<m_iNumVertices; i++)
	{
		for( iAxis=0; iAxis<3; iAxis++ )
		{
			if( m_pavVertex[i][iAxis] < vMin[iAxis] )
				vMin[iAxis] = m_pavVertex[i][iAxis];

			if( vMax[iAxis] < m_pavVertex[i][iAxis] )
				vMax[iAxis] = m_pavVertex[i][iAxis];
		}
	}

	m_AABB.SetMaxAndMin( vMax, vMin );
}


void ConvexPolygon::SetNormal( Vector3& rvNormal )
{
	m_Plane.normal = rvNormal;
///	m_vNormal.setValue( (float *)&rvNormal );
}


void ConvexPolygon::SetDistance( Scalar fDist )
{
	m_Plane.dist = fDist;
}


Vector3 ConvexPolygon::GetSupportPoint(const Vector3& v) const
{
	Scalar d, max_dot = -99999;
	int i, iNumVertices = m_iNumVertices; 
	int iMax = 0;
	for(i=0; i<iNumVertices; i++)
	{
		d = Vec3Dot( m_pavVertex[i], v );
		if( max_dot < d )
		{
			max_dot = d;
			iMax = i;
		}
	}
	return m_pavVertex[iMax];

}


void ConvexPolygon::LoadFromFile( FILE *fp )
{
	fread( &m_iNumVertices, sizeof(int), 1, fp );

	SafeDeleteArray( m_pavVertex );
	m_pavVertex = new Vector3 [m_iNumVertices];
	fread( m_pavVertex, sizeof(Vector3), m_iNumVertices, fp );

	fread( &m_Plane, sizeof(SPlane), 1, fp );

	fread( &m_iPolygonIndex, sizeof(int), 1, fp );

	fread( &m_uiCollisionFlag, sizeof(int), 1, fp );

	fread( &m_AABB, sizeof(AABB3), 1, fp );
}


void ConvexPolygon::WriteToFile( FILE *fp )
{
	fwrite( &m_iNumVertices, sizeof(int), 1, fp );
	fwrite( m_pavVertex, sizeof(Vector3), m_iNumVertices, fp );

	fwrite( &m_Plane, sizeof(SPlane), 1, fp );

	fwrite( &m_iPolygonIndex, sizeof(int), 1, fp );

	fwrite( &m_uiCollisionFlag, sizeof(int), 1, fp );

	fwrite( &m_AABB, sizeof(AABB3), 1, fp );
}



void ConvexPolygon::SetVertexCollisionFlag(int iIndex, bool bCheckCollision)
{
	if( bCheckCollision )
	{	// raise the flag
		m_uiCollisionFlag |= ( 1 << iIndex );
	}
	else
	{	// clear the flag
		m_uiCollisionFlag &= ~( 1 << iIndex );
	}
}


void ConvexPolygon::SetEdgeCollisionFlag(int iIndex, bool bCheckCollision)
{
	if( bCheckCollision )
	{	// raise the flag
		m_uiCollisionFlag |= ( 1 << (iIndex + EDGE_COLLISION_FLAG_OFFSET) );
	}
	else
	{	// clear the flag
		m_uiCollisionFlag &= ~( 1 << (iIndex + EDGE_COLLISION_FLAG_OFFSET) );
	}
}

} // namespace amorphous
