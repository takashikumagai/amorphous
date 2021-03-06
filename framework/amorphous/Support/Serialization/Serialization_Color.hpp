#ifndef __KGL_COLOR_SERIALIZATION_H__
#define __KGL_COLOR_SERIALIZATION_H__


#include "../../Graphics/FloatRGBColor.hpp"
#include "../../Graphics/FloatRGBAColor.hpp"

#include "Archive.hpp"


namespace amorphous
{

namespace serialization
{


inline IArchive& operator & ( IArchive& ar, SFloatRGBColor& color )
{
	ar & color.red;
	ar & color.green;
	ar & color.blue;

	return ar;
}


inline IArchive& operator & ( IArchive& ar, SFloatRGBAColor& color )
{
	ar & color.red;
	ar & color.green;
	ar & color.blue;
	ar & color.alpha;

	return ar;
}


} // namespace serialization

} // namespace amorphous


#endif  /*  __KGL_COLOR_SERIALIZATION_H__  */
