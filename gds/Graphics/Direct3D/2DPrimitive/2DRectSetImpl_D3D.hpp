#ifndef  __2DRectSetImpl_D3D_HPP__
#define  __2DRectSetImpl_D3D_HPP__


#include "../../base.hpp"
#include "3DMath/Vector2.hpp"
#include "3DMath/Vector3.hpp"
#include "Graphics/FVF_TLVertex.h"
#include "Graphics/RectTriListIndex.hpp"
#include "Graphics/Direct3D9.hpp"
#include "Graphics/2DPrimitive/2DPrimitive.hpp"
#include "Graphics/2DPrimitive/2DRectSet.hpp"
#include "Graphics/TextureHandle.hpp"

#include <vector>


//=============================================================================
// C2DRectSetImpl_D3D
//=============================================================================

class C2DRectSetImpl_D3D : public C2DRectSetImpl
{
	std::vector<TLVERTEX> m_vecRectVertex;

public:

	/// position

	inline virtual void SetRectMinMax( int rect_index, const Vector2& vMin, const Vector2& vMax );

//	inline virtual void SetRectMinMax( int rect_index, const Vector3& vMin, const Vector3& vMax );

	inline virtual void SetRectMinMax( int rect_index, float min_x, float min_y, float max_x, float max_y );

	inline virtual void SetRectVertexPosition( int rect_index, int vert_index, const Vector2& vPos );

	inline virtual void SetRectVertexPosition( int rect_index, int vert_index, const Vector3& vPos );

	inline virtual void SetRectVertexPosition( int rect_index, int vert_index, const float x, const float y );

	inline virtual void SetVertexPosition( int vert_index, const float x, const float y );

	inline virtual Vector2 GetRectVertexPosition( int rect_index, int vert_index );


	/// texture coord

	inline virtual void SetTextureCoordMinMax( int rect_index, float u_min, float v_min, float u_max, float v_max );

	inline virtual void SetTextureCoordMinMax( int rect_index, const TEXCOORD2& vMin, const TEXCOORD2& vMax );

	inline virtual void SetRectVertexTextureCoord( int rect_index, int vert_index, float u, float v );

	inline virtual TEXCOORD2 GetTopLeftTextureCoord( int rect_index );

	inline virtual TEXCOORD2 GetBottomRightTextureCoord( int rect_index );


	/// color

	inline virtual void SetColor( U32 color );

	inline virtual void SetRectColor( int rect_index, U32 color );

	inline virtual void SetRectVertexColor( int rect_index, int vert_index, const SFloatRGBAColor& color );


	/// FVF vertex

//	inline virtual void SetRectVertex( int rect_index, int vert_index, const TLVERTEX& vSrc );

//	inline virtual void SetVertex( int vert_index, const TLVERTEX& vSrc );


	inline virtual int GetNumRects() { return (int)m_vecRectVertex.size() / 4; }

	inline virtual void SetNumRects( int num_rects );

	inline virtual void AddRects( int num_rects );

	inline virtual void ClearRects() { m_vecRectVertex.resize(0); }

	inline virtual void draw( int start_rect_index, int num_rects );

	/// renders all the rects with the current render states
//	inline virtual void draw() { draw( 0, (int)m_vecRectVertex.size() / 4 ); }

	inline virtual void Draw( int start_rect_index, int num_rects );

	/// renders all the rects
//	inline virtual void Draw() { Draw( 0, (int)m_vecRectVertex.size() / 4 ); }

	inline virtual void Draw( int start_rect_index, int num_rects, const CTextureHandle& texture );

//	inline virtual void Draw( const CTextureHandle& texture ) { Draw( 0, (int)m_vecRectVertex.size() / 4, texture ); }
};


//================================/ inline implementations /================================//


inline void C2DRectSetImpl_D3D::SetRectMinMax( int rect_index,
									   float min_x, float min_y,
									   float max_x, float max_y )
{
	const int offset = rect_index * 4;
	m_vecRectVertex[offset    ].vPosition.x = min_x;
	m_vecRectVertex[offset    ].vPosition.y = min_y;

	m_vecRectVertex[offset + 1].vPosition.x = max_x;
	m_vecRectVertex[offset + 1].vPosition.y = min_y;

	m_vecRectVertex[offset + 2].vPosition.x = max_x;
	m_vecRectVertex[offset + 2].vPosition.y = max_y;

	m_vecRectVertex[offset + 3].vPosition.x = min_x;
	m_vecRectVertex[offset + 3].vPosition.y = max_y;
}


inline void C2DRectSetImpl_D3D::SetRectMinMax( int rect_index,
									   const Vector2& vMin,
									   const Vector2& vMax )
{
	SetRectMinMax( rect_index, vMin.x, vMin.y, vMax.x, vMax.y );
}


inline void C2DRectSetImpl_D3D::SetRectVertexPosition( int rect_index, int vert_index, const Vector2& vPos )
{
	m_vecRectVertex[rect_index*4+vert_index].vPosition.x = vPos.x;
	m_vecRectVertex[rect_index*4+vert_index].vPosition.y = vPos.y;
}


inline void C2DRectSetImpl_D3D::SetRectVertexPosition( int rect_index, int vert_index, const Vector3& vPos )
{
	m_vecRectVertex[rect_index*4+vert_index].vPosition = vPos;
}


inline void C2DRectSetImpl_D3D::SetRectVertexPosition( int rect_index, int vert_index, const float x, const float y )
{
	m_vecRectVertex[rect_index*4+vert_index].vPosition.x = x;
	m_vecRectVertex[rect_index*4+vert_index].vPosition.y = y;
}


inline void C2DRectSetImpl_D3D::SetVertexPosition( int vert_index, const float x, const float y )
{
	m_vecRectVertex[vert_index].vPosition.x = x;
	m_vecRectVertex[vert_index].vPosition.y = y;
}


inline Vector2 C2DRectSetImpl_D3D::GetRectVertexPosition( int rect_index, int vert_index )
{
	D3DXVECTOR2 v = m_vecRectVertex[rect_index * 4 + vert_index].vPosition;
	return Vector2( v.x, v.y );
}


inline void C2DRectSetImpl_D3D::SetTextureCoordMinMax( int rect_index, float u_min, float v_min, float u_max, float v_max )
{
	const int offset = rect_index * 4;
	m_vecRectVertex[offset    ].tu = u_min;
	m_vecRectVertex[offset    ].tv = v_min;

	m_vecRectVertex[offset + 1].tu = u_max;
	m_vecRectVertex[offset + 1].tv = v_min;

	m_vecRectVertex[offset + 2].tu = u_max;
	m_vecRectVertex[offset + 2].tv = v_max;

	m_vecRectVertex[offset + 3].tu = u_min;
	m_vecRectVertex[offset + 3].tv = v_max;
}


inline void C2DRectSetImpl_D3D::SetTextureCoordMinMax( int rect_index, const TEXCOORD2& vMin, const TEXCOORD2& vMax )
{
	SetTextureCoordMinMax( rect_index, vMin.u, vMin.v, vMax.u, vMax.v );
}


inline void C2DRectSetImpl_D3D::SetRectVertexTextureCoord( int rect_index, int vert_index, float u, float v )
{
	const int index = rect_index*4 + vert_index;
	m_vecRectVertex[index].tu = u;
	m_vecRectVertex[index].tv = v;
}


inline TEXCOORD2 C2DRectSetImpl_D3D::GetTopLeftTextureCoord( int rect_index )
{
	TLVERTEX v = m_vecRectVertex[rect_index*4];
	return TEXCOORD2( v.tu, v.tv );
}


inline TEXCOORD2 C2DRectSetImpl_D3D::GetBottomRightTextureCoord( int rect_index )
{
	TLVERTEX v = m_vecRectVertex[rect_index*4 + 2]; // 3rd vertex is at the bottom right corner
	return TEXCOORD2( v.tu, v.tv );
}


inline void C2DRectSetImpl_D3D::SetColor( U32 color )
{
	const size_t num_vertices = m_vecRectVertex.size();
	for(size_t i=0; i<num_vertices; i++)
		m_vecRectVertex[i].color = color;
}


inline void C2DRectSetImpl_D3D::SetRectColor( int rect_index, U32 color )
{
	m_vecRectVertex[rect_index*4  ].color = color;
	m_vecRectVertex[rect_index*4+1].color = color;
	m_vecRectVertex[rect_index*4+2].color = color;
	m_vecRectVertex[rect_index*4+3].color = color;
}


inline void C2DRectSetImpl_D3D::SetRectVertexColor( int rect_index, int vert_index, const SFloatRGBAColor& color )
{
	U32 argb32 = color.GetARGB32();
	m_vecRectVertex[rect_index*4 + vert_index].color = argb32;
}


inline void C2DRectSetImpl_D3D::SetNumRects( int num_rects )
{
	const int num_vertices = num_rects * 4;

	TLVERTEX v;
	v.rhw = 1.0f;
	v.vPosition.z = 0.0f;
//	v.color = 0xFF000000;

	m_vecRectVertex.resize( num_vertices, v );
}


inline void C2DRectSetImpl_D3D::AddRects( int num_rects )
{
	const int num_vertices = num_rects * 4;

	TLVERTEX v;
	v.rhw = 1.0f;
	v.vPosition.z = 0.0f;
//	v.color = 0xFF000000;

	for( int i=0; i<num_vertices; i++ )
		m_vecRectVertex.push_back( v );
}


inline void C2DRectSetImpl_D3D::Draw( int start_rect_index, int num_rects )
{
//	SetBasicRenderStates();

//	SetStdRenderStates();
	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();

	// no texture
	// - use only the vertex color & alpha
	pd3dDev->SetTexture( 0, NULL );

	pd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pd3dDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );

	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pd3dDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	draw( start_rect_index, num_rects );
}


inline void C2DRectSetImpl_D3D::Draw( int start_rect_index, int num_rects, const CTextureHandle& texture )
{
	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();

//	SetBasicRenderStates();

	pd3dDev->SetTexture( 0, texture.GetTexture() );

	// color blend settings
	pd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//	pd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	pd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pd3dDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );

	// alpha blend settings
	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
	pd3dDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	// draw rectangles
	draw( start_rect_index, num_rects );

//	if( FAILED(hr) )
//		MessageBox(NULL, "DrawPrimUP failed.", "Error", MB_OK);
}


inline void C2DRectSetImpl_D3D::draw( int start_rect_index, int num_rects )
{
	if( num_rects == 0 )
		return;

	DIRECT3D9.GetDevice()->SetFVF( D3DFVF_TLVERTEX );

	DIRECT3D9.GetDevice()->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST,
									0,
									num_rects * 4,
									num_rects * 2,
									s_RectTriListIndex,
									D3DFMT_INDEX16,
									&(m_vecRectVertex[start_rect_index*4]),
									sizeof(TLVERTEX) );

}



#endif  /*  __2DRectSetImpl_D3D_HPP__  */
