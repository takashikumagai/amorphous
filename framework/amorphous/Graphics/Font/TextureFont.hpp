#ifndef  __TEXTUREFONT_H__
#define  __TEXTUREFONT_H__

#include "FontBase.hpp"
#include "../fwd.hpp"
#include "../TextureHandle.hpp"
#include "../TextureCoord.hpp"
#include "../2DPrimitive/2DRectSet.hpp"
#include "amorphous/3DMath/AABB2.hpp"
#include "amorphous/Support/Serialization/Serialization.hpp"
#include "amorphous/Support/Serialization/Serialization_3DCommon.hpp"
#include "amorphous/Support/Serialization/Serialization_3DMath.hpp"


namespace amorphous
{
using namespace serialization;


extern void SetRenderStatesForTextureFont( AlphaBlend::Mode dest_alpha_blend );


class TextureFont : public FontBase
{
public:

	class CharRect : public IArchiveObjectBase
	{
	public:

		TEXCOORD2 tex_min, tex_max;
		AABB2 rect;

		/// horizontal advance for this character
		float advance;

		void Serialize( IArchive& ar, const unsigned int version )
		{
			ar & tex_min & tex_max;
			ar & rect;
			ar & advance;
		}
	};

	enum Params
	{
		NUM_MAX_LETTERS = 0x1000,
		CHAR_OFFSET = ' '
	};
	
	static const std::string ms_Characters;

	static bool ms_ProfileTextureFont;

protected:

	std::string m_strTextureFilename;

	TextureHandle m_FontTexture;

	std::vector<CharRect> m_vecCharRect;

	int m_BaseHeight;

	/// the number of divisions of the texture
	int m_NumTexDivisionsX;
	int m_NumTexDivisionsY;

	C2DRectSet m_TextBox;

	C2DRectSet m_ShadowTextBox;

	/// used to store a group of text and draw them
	/// with a single DrawPrimitiveUP() call
	int m_CacheIndex;

	float m_fItalic;	///< 1.0f == a letter slant by the width of one char

	Vector2 m_vShadowShift;

	SFloatRGBAColor m_ShadowColor;

protected:

	void InitInternal();

	bool InitCharacterRects();

public:

	TextureFont();

	TextureFont( const std::string texture_filename,
		          int font_width, int font_height,
		          int num_tex_divisions_x = 16, int num_tex_divisions_y = 8 );

	virtual ~TextureFont();

	void Release();

	void Reload();

	/// InitFont() is not made virtual and completely different from that of CFont
	bool InitFont( const std::string texture_filename,
		           int font_width, int font_height,
		           int num_tex_divisions_x = 16, int num_tex_divisions_y = 8 );

	bool InitFont( const SimpleBitmapFontData& bitmap );

	bool InitFont( const SimpleBitmapFontData *pBitmap ) { return pBitmap ? InitFont(*pBitmap) : false; }

	virtual void SetFontSize(int FontWidth, int FontHeight);

	virtual float GetHorizontalFactor() const { return (float)m_FontWidth; }
	virtual float GetVerticalFactor() const   { return (float)m_FontHeight; }

	//	virtual void DrawText( const char* pcStr, Vector2 vMin, Vector2 vMax);

	virtual void DrawText( const char* pcStr, const Vector2& vPos, U32 dwColor );

	virtual void DrawText( const char* pcStr, const Vector2& vPos, const Vector2& vPivotPoint, float rotation_angle, U32 dwColor );

	int GetTextWidth( const char *text ) const;

	/// push text data to the buffer
	void CacheText( const char* pcStr, const Vector2& vPos, const Vector2& vPivotPoint, float rotation_angle, U32 dwColor );

	inline void CacheText( const char* pcStr, const Vector2& vPos, const Vector2& vPivotPoint = Vector2(0,0), float rotation_angle = 0.0f )
	{
		CacheText( pcStr, vPos, vPivotPoint, rotation_angle, m_dwFontColor );
	}

	/// draw text in the buffer
	void DrawCachedText();

	/// clear text in the buffer
	inline void ClearCache() { m_CacheIndex = 0; }

	/// could be of some service if you are using fixed function pipeline...
//	void SetDefaultTextureStageStates();

	void SetItalic( float italic_weight ) { m_fItalic = italic_weight; }

	void SetShadowColor( const SFloatRGBAColor& shadow_color ) { m_ShadowColor = shadow_color; }

	/// Sets the distance between shadow text and the body text
	void SetShadowShift( Vector2 vShift ) { m_vShadowShift = vShift; }

	void SetShadowShift( int x, int y ) { m_vShadowShift = Vector2((float)x,(float)y); }

	virtual int GetFontType() const { return FONTTYPE_TEXTURE; }

	static void ProfileTextureFont( bool enable ) { ms_ProfileTextureFont = enable; }
};

} // namespace amorphous



#endif		/*  __TEXTUREFONT_H__  */
