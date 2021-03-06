#ifndef __KGL_2DARRAY_SERIALIZATION_H__
#define __KGL_2DARRAY_SERIALIZATION_H__


#include "../array2d.hpp"
#include "Archive.hpp"


namespace amorphous
{

namespace serialization
{


template <class T>
inline IArchive& operator & ( IArchive& ar, array2d<T>& r2DArray )
{
	int x, y, iSizeX, iSizeY;
	if( ar.GetMode() == IArchive::MODE_OUTPUT )
	{
		// record array size
		iSizeX = r2DArray.size_x();
		iSizeY = r2DArray.size_y();
		ar & iSizeX;
		ar & iSizeY;

		// record each element
		for( y=0; y<iSizeY; y++ )
		{
			for( x=0; x<iSizeX; x++ )
			{
				ar & r2DArray(x,y);
			}
		}
	}
	else // i.e. input mode
	{
		// load array size
		ar & iSizeX;
		ar & iSizeY;
		r2DArray.resize(iSizeX,iSizeY);

		for( y=0; y<iSizeY; y++ )
		{
			for( x=0; x<iSizeX; x++ )
			{
				ar & r2DArray(x,y);
			}
		}
	}

	return ar;
}


} // namespace serialization

} // namespace amorphous


#endif  /*  __KGL_2DARRAY_SERIALIZATION_H__  */
