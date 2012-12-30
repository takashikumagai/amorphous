#ifndef __FVFTEXTUREVERTEX_H__
#define __FVFTEXTUREVERTEX_H__


#include <d3dx9.h>
#include "../TextureCoord.hpp"


namespace amorphous
{
	
struct TEXTUREVERTEX
{
	D3DXVECTOR3 vPosition;

	D3DCOLOR color;

	TEXCOORD2 tex;


	enum eFVF
	{
		FVF = (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
	};

//	static const D3DVERTEXELEMENT9 VertexElements[4];
};


#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)


const D3DVERTEXELEMENT9 TEXTUREVERTEX_DECLARATION[4] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
	{ 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

/*
const D3DVERTEXELEMENT9 TEXTUREVERTEX::VertexElements[4] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
	{ 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};*/

//extern const D3DVERTEXELEMENT9 TEXTUREVERTEX_DECLARATION[4];

} // amorphous


#endif	/* __FVFTEXTUREVERTEX_H__ */
