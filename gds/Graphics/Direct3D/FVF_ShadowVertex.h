#ifndef __FVF_SHADOW_VERTEX_H__
#define __FVF_SHADOW_VERTEX_H__


#include <d3dx9.h>


namespace amorphous
{
	
struct SHADOWVERTEX
{
    D3DXVECTOR3 vPosition;

	D3DXVECTOR3 vNormal;


	enum eFVF
	{
		FVF = (D3DFVF_XYZ|D3DFVF_NORMAL)
	};

//	const static D3DVERTEXELEMENT9 Decl[3];

};


//const D3DVERTEXELEMENT9 SHADOWVERT::Decl[3] =
const D3DVERTEXELEMENT9 SHADOWVERTEX_DECLARATION[3] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
    D3DDECL_END()
};

} // amorphous


#endif  /*  __FVF_SHADOW_VERTEX_H__  */


