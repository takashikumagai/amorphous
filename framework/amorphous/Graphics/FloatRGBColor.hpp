#ifndef __FLOATRGBCOLOR_H__
#define __FLOATRGBCOLOR_H__


#include "amorphous/base.hpp"


namespace amorphous
{


struct SFloatRGBColor
{
	float red;
	float green;
	float blue;

	inline SFloatRGBColor operator *(const float f) const;
	inline SFloatRGBColor operator /(const float f) const;
	inline SFloatRGBColor operator +(const SFloatRGBColor& rColor) const;
	inline SFloatRGBColor operator -(const SFloatRGBColor& rColor) const;
	inline SFloatRGBColor operator +=(const SFloatRGBColor& rColor);
	inline SFloatRGBColor operator -=(const SFloatRGBColor& rColor);
	inline bool operator==( const SFloatRGBColor& rhs ) const;
	inline bool operator!=( const SFloatRGBColor& rhs ) const;

	/// \brief Do multiplication operations for each component
	/// - same as '*' in programmable shader
	inline SFloatRGBColor operator *(const SFloatRGBColor& rColor) const;

//	inline void CopyFromD3DCOLOR(D3DCOLOR color);
	/// set color expressed as 32-bit integer with 8-bit for each component
	inline void SetARGB32( U32 color );

	/// alpha is set to 255(0xFF)
	inline U32 GetARGB32() const;

	inline U8 GetRedByte()   const { return (U8)get_clamped( (U32)(red   * 256.0f), (U32)0, (U32)255 ); }
	inline U8 GetGreenByte() const { return (U8)get_clamped( (U32)(green * 256.0f), (U32)0, (U32)255 ); }
	inline U8 GetBlueByte()  const { return (U8)get_clamped( (U32)(blue  * 256.0f), (U32)0, (U32)255 ); }

	inline SFloatRGBColor();

	inline SFloatRGBColor( float r, float g, float b );

	inline void SetRGB( float r, float g, float b );

	void SetToWhite()   { *this = White(); }
	void SetToBlack()   { *this = Black(); }
	void SetToRed()     { *this = Red(); }
	void SetToGreen()   { *this = Green(); }
	void SetToBlue()    { *this = Blue(); }
	void SetToYellow()  { *this = Yellow(); }
	void SetToMagenta() { *this = Magenta(); }
	void SetToAqua()    { *this = Aqua(); }

	static const SFloatRGBColor White()   { return SFloatRGBColor( 1.0f, 1.0f, 1.0f ); }
	static const SFloatRGBColor Black()   { return SFloatRGBColor( 0.0f, 0.0f, 0.0f ); }
	static const SFloatRGBColor Red()     { return SFloatRGBColor( 1.0f, 0.0f, 0.0f ); }
	static const SFloatRGBColor Green()   { return SFloatRGBColor( 0.0f, 1.0f, 0.0f ); }
	static const SFloatRGBColor Blue()    { return SFloatRGBColor( 0.0f, 0.0f, 1.0f ); }
	static const SFloatRGBColor Yellow()  { return SFloatRGBColor( 1.0f, 1.0f, 0.0f ); }
	static const SFloatRGBColor Magenta() { return SFloatRGBColor( 1.0f, 0.0f, 1.0f ); }
	static const SFloatRGBColor Aqua()    { return SFloatRGBColor( 0.0f, 1.0f, 1.0f ); }
};


} // namespace amorphous


#include "FloatRGBColor.inl"



#endif  /*  __FLOATRGBCOLOR_H__  */
