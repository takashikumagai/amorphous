#ifndef __FontFactory_HPP__
#define __FontFactory_HPP__


#include <string>
#include "Graphics/fwd.hpp"
#include "FontBase.hpp"


class CFontFactory
{
public:

	CFontBase* CreateFontRawPtr( CFontBase::FontType type );

	/// Create a font
	/// Determines the font class from the font_name
	/// If font_name is a pathname of an image file, CTextureFont is created
	/// and the image file is loaded as a texture that contains the fixed-pitch ascii characters.
	/// If font_name is a *.ttf or *.otf file, CUTFFont is created.
	/// If font_name begins with the string, "BuiltinFont::" CTextureFont is created
	/// and the specified built-in font is loaded
	CFontBase* CreateFontRawPtr( const std::string& font_name, int font_width = 16, int font_height = 32 );

	/// returns a owned ref
	CFontBase* CreateFontRawPtr( CFontBase::FontType type, const std::string& font_name, int font_width, int font_height );
};



#endif /* __FontFactory_HPP__ */
