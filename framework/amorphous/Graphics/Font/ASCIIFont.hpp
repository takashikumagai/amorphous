#ifndef  __amorphous_ASCIIFont_HPP__
#define  __amorphous_ASCIIFont_HPP__


#include "TextureFont.hpp"
#include "amorphous/Support/array2d.hpp"
#include "amorphous/Support/ImageArchive.hpp"


namespace amorphous
{

class FontTextureLoader;


/**
  - archived resource of ANSI texture font
    - filename suffix: ".tfa"
    - Created when a texture font is initialized for the first time
	  - Subsequent init call of the same font will try to load the font
	    from the texture font archive, hopefully reducing the init time
		- i.e. Instance of this class works as a cache of font texture resource.

  Rationale to have texture font archive class
  - to support feature to initialize font texture quickly
    - no archived resource -> need to
	  - render the glyphs to buffer.
	  - copy the buffer content to memory of the texture resource
	  - usually,
	    - color channel(rgb): full brightness
		- alpha cahnnel: grayscale values of font

		=========================================================
		Cannot load texture from image archive directly!!!
		Need to be (binary db filepath)::(keyname)
		=========================================================

    - As such, the texture image is saved as a separate data in a database of CBinaryDatabase<std::string>
      because the Direct3D should be able to separately load the image after releasing
	  and re-creating the D3D device object.

  - Why a separate archive class? What about deriving font class from IArchiveObjectBase?
    - FontBase would need to be an archive object to avoid mulitple inheritance.
*/
class TextureFontArchive : public IArchiveObjectBase
{
public:

	int BaseCharHeight;

	std::vector<TextureFont::CharRect> vecCharRect;

public:

	TextureFontArchive() : BaseCharHeight(64) {}

	void Serialize( IArchive& ar, const unsigned int version )
	{
		ar & BaseCharHeight & vecCharRect;
	}
};


/**
 A texture font created from TrueType font (*.ttf)
  - Supports the ASCII characters
  - Creates a texture that stores the rendered ASCII characters
  - Saves the texture to disk to use the texture font by simply loading it
    from the next time

*/
class ASCIIFont : public TextureFont
{
private:

	std::string m_FontFilepath;

	std::shared_ptr<FontTextureLoader> m_pTextureLoader;

	static std::string ms_TextureArchiveDestDirectoryPath;

private:

	void InitTrueTypeFontInternal();

	/// Creates a bitmap that contains ASCII characters
	bool CreateFontTextureFromTrueTypeFont( array2d<U8>& dest_bitmap_buffer );

public:

	ASCIIFont();

	ASCIIFont( const std::string& filename,
		          int resolution = 64, int font_width = 0, int font_height = 32 );

	virtual ~ASCIIFont();

//	void Release();

//	void Reload();

	/// InitFont() is not made virtual and completely different from that of CFont
	/// \param height font height
	/// \param width ignored for now. width is determined from the height
	/// \param filename font file. Filetype could be one of the following
	/// - .ttf TrueType font
	/// - .otf OpenType font
	/// - .tfd ???
	bool InitFont( const std::string& filename, int resolution,
		           int font_width, int font_height );

	static void SetTextureArchiveDestDirectoryPath( const std::string& dir_path ) { ms_TextureArchiveDestDirectoryPath = dir_path; }

//	virtual void SetFontSize(int FontWidth, int FontHeight);

	virtual float GetHorizontalFactor() const { return GetVerticalFactor(); }
	virtual float GetVerticalFactor() const   { return (float)m_FontHeight / (float)m_BaseHeight; }

//	virtual void DrawText( const char* pcStr, const Vector2& vPos, U32 dwColor );

	virtual int GetFontType() const { return FONTTYPE_TRUETYPETEXTURE; }

	bool SaveTextureAndCharacterSet( const std::string& filepath ); 

	bool SaveTextureFontArchive( const std::string& pathname );

	static const char *GetTextureFontArchiveExtension() { return "tfd"; }

	friend class FontTextureLoader;
};


//typedef ASCIIFont TrueTypeTextureFont;


} // namespace amorphous



#endif		/*  __amorphous_ASCIIFont_HPP__  */
