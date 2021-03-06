#include "D3DXFont.hpp"
#include "Graphics/Direct3D/Direct3D9.hpp"
#include "Support/Log/DefaultLog.hpp"
#include "Support/StringAux.hpp"


namespace amorphous
{


using namespace std;


CD3DXFont::CD3DXFont( const std::string font_name, int font_width, int font_height, int type_flag )
:
m_pD3DXFont(NULL)
{
	InitFont( font_name, font_width, font_height );
}


CD3DXFont::~CD3DXFont()
{
	Release();
}


int CD3DXFont::InitFont( const std::string font_name, int font_width, int font_height, int type_flag )
{
	LPD3DXFONT pTempD3DXFont;

	if( font_width <= 0 || font_height <= 0 )
	{
		LOG_PRINT_ERROR( fmt_string("invalid font size: %d(w) x %d(h)", font_width, font_height) );
		return 0;
	}

	LOG_PRINT( fmt_string( " - Creating a font: '%s', size: %d x %d", font_name.c_str(), font_width, font_height ) );

	if( !DIRECT3D9.GetDevice() )
		LOG_PRINT_ERROR( " - D3D has not been initialized." );

	UINT weight = m_TypeFlag & FontBase::BOLD ? 1000 : 0;

	HRESULT hr = D3DXCreateFont(DIRECT3D9.GetDevice(),
		                        font_height,
								font_width,
								weight,
								0,
								m_TypeFlag & FontBase::ITALIC ? TRUE : FALSE,
								SHIFTJIS_CHARSET, 0, 0, 0,
								font_name.c_str(),
								&pTempD3DXFont);
	if( FAILED(hr) )
	{
		LOG_PRINT_ERROR( " - D3DXCreateFont failed." );
		return 0;
	}
	else
	{	// release current font (if loaded) & set the new font 
		SAFE_RELEASE( m_pD3DXFont );
		m_pD3DXFont = pTempD3DXFont;

		m_strFontName = font_name;
		m_FontWidth  = font_width;
		m_FontHeight = font_height;
	}

	return 1;
}


void CD3DXFont::Release()
{
	SAFE_RELEASE( m_pD3DXFont );
}


void CD3DXFont::Reload()
{
	InitFont( m_strFontName, m_FontWidth, m_FontHeight );
}


void CD3DXFont::SetFontSize(int font_width, int font_height)
{
	if( m_FontWidth == font_width && m_FontHeight == font_height )
		return;

	// need to release and recreate font object
	Release();

	InitFont( m_strFontName, font_width, font_height );
}



#define FONT_MAX_WIDTH	1280
#define FONT_MAX_HEIGHT	1024


void CD3DXFont::DrawText(const char *pStr, const Vector2& vPos, U32 color)
{
	if( !m_pD3DXFont )
		return;

	RECT Rect;
	Rect.left = (DWORD)vPos.x;
	Rect.top = (DWORD)vPos.y;
	Rect.right = FONT_MAX_WIDTH;
	Rect.bottom = FONT_MAX_HEIGHT;

	DIRECT3D9.GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	m_pD3DXFont->DrawText(NULL, pStr, -1, &Rect, DT_NOCLIP, color);

}


/*
void CD3DXFont::DrawText( const char *str, const Vector2& vPos, DWORD color )
{
	if( 1023 <= strlen(str) )
	{
		MessageBox( NULL, "text buffer overflow", "CD3DXFont::DrawText() error", MB_OK|MB_ICONWARNING);
		return;
	}

	char acTempStr[1024];
	strcpy( acTempStr, str );
	DrawText( acTempStr, vPos, color );
}*/


} // namespace amorphous
