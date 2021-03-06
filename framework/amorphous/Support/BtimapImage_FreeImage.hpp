#ifndef __amorphous_BitmapImage_FreeImage_HPP__
#define __amorphous_BitmapImage_FreeImage_HPP__


#include <memory>
#include "FreeImage.h" // FreeImage library header

#include "stream_buffer.hpp"

// Comment out this header inclusion and do the following replacings
// if you want to use BitmapImage class without the log system.
// 1. Define the following macros
//   #define LOG_PRINT(x)         std::cout << x
//   #define LOG_PRINT_VERBOSE(x) std::cout << x
//   #define LOG_PRINT_ERROR(x)   std::cout << x
// 2. Replace 'g_Log.Print(x)' with 'printf(x)'.
#include "Log/DefaultLog.hpp"

#pragma comment( lib, "FreeImage.lib" )


namespace amorphous
{
	using std::shared_ptr;

	class BitmapImage;

	inline unsigned int DLL_CALLCONVImageReadProc( void *buffer, unsigned int size, unsigned count, fi_handle handle );
	inline unsigned int DLL_CALLCONV ImageWriteProc( void *buffer, unsigned size, unsigned int count, fi_handle handle );
	inline int DLL_CALLCONV ImageSeekProc( fi_handle handle, long offset, int origin );
	inline long DLL_CALLCONV ImageTellProc( fi_handle handle );


	class ImageStreamBufferHolder
	{
		stream_buffer *m_pStreamBuffer;

	public:

		friend class BitmapImage;
		friend unsigned int DLL_CALLCONV ImageReadProc( void *buffer, unsigned int size, unsigned count, fi_handle handle );
		friend unsigned int DLL_CALLCONV ImageWriteProc( void *buffer, unsigned size, unsigned int count, fi_handle handle );
		friend int DLL_CALLCONV ImageSeekProc( fi_handle handle, long offset, int origin );
		friend long DLL_CALLCONV ImageTellProc( fi_handle handle );
	};


	/// The singleton instance of ImageStreamBufferHolder
	inline ImageStreamBufferHolder& GetImageStreamBufferHolder()
	{
		static ImageStreamBufferHolder holder;
		return holder;
	}


	inline unsigned int DLL_CALLCONV ImageReadProc( void *buffer, unsigned int size, unsigned int count, fi_handle handle )
	{
		return GetImageStreamBufferHolder().m_pStreamBuffer->read( buffer, size * count );
	}

	inline unsigned int DLL_CALLCONV ImageWriteProc( void *buffer, unsigned int size, unsigned int count, fi_handle handle )
	{
		return GetImageStreamBufferHolder().m_pStreamBuffer->write( buffer, size * count );
	}

	inline int DLL_CALLCONV ImageSeekProc( fi_handle handle, long offset, int origin )
	{
		stream_buffer *pStreamBuffer = GetImageStreamBufferHolder().m_pStreamBuffer;
		switch(origin)
		{
		case SEEK_SET: pStreamBuffer->seek_pos( offset ); break;
		case SEEK_CUR: pStreamBuffer->seek_pos( pStreamBuffer->get_current_pos() + offset ); break;
		case SEEK_END: pStreamBuffer->seek_pos( (int)pStreamBuffer->buffer().size() + offset ); break;
		default:
			return 0;
		}

		return 0; // assume it went successful
	}

	inline long DLL_CALLCONV ImageTellProc( fi_handle handle )
	{
		return (long)GetImageStreamBufferHolder().m_pStreamBuffer->get_current_pos();
	}


	inline FREE_IMAGE_FORMAT ToFIF( const std::string& image_ext )
	{
		if( image_ext == "png" )       return FIF_PNG;
		else if( image_ext == "jpg" )  return FIF_JPEG;
		else if( image_ext == "tga" )  return FIF_TARGA;
		else if( image_ext == "tif" )  return FIF_TIFF;
		else if( image_ext == "tiff" ) return FIF_TIFF;
		else if( image_ext == "hdr" )  return FIF_HDR;
		else if( image_ext == "bmp" )  return FIF_BMP;
		else if( image_ext == "dds" )  return FIF_DDS;
		//	else if( image_ext == "" )    return ;
		//	else if( image_ext == "" )    return ;

		return FIF_UNKNOWN;
	}


	/**
	- Loads image form files (.bmp, .jpg, etc.)
	- Uses FreeImage library

	*/
	class BitmapImage
	{
		FIBITMAP* m_pFreeImageBitMap;

		int m_BitsPerPixel; ///< the size of one pixel in the bitmap in bits

	private:

		BitmapImage( FIBITMAP *pFreeImageBitMap )
			:
			m_pFreeImageBitMap(NULL),
			m_BitsPerPixel(0)
		{
			Reset(pFreeImageBitMap);
		}

		void Reset( FIBITMAP *pFreeImageBitMap )
		{
			Release();
			m_pFreeImageBitMap = pFreeImageBitMap;
			if( m_pFreeImageBitMap )
				m_BitsPerPixel = FreeImage_GetBPP( m_pFreeImageBitMap );
		}

		void Release()
		{
			if( m_pFreeImageBitMap )
			{
				FreeImage_Unload( m_pFreeImageBitMap );
				m_pFreeImageBitMap = NULL;
				m_BitsPerPixel = 0;
			}
		}

		inline bool Save32BPPImageTo24JPEGFile( const std::string& image_file_pathname, int flag );

	public:

		BitmapImage() : m_pFreeImageBitMap(NULL), m_BitsPerPixel(0) {}

		inline BitmapImage( int width, int height, int bpp );

		/// \param bpp bits per pixel. Must support RGBA format
		//	inline BitmapImage( int width, int height, int bpp, const RGBAColorType& color );

		/// TODO: Make the argument const
		/// - Need to do sth to ImageStreamBufferHolder. See the function definition
		inline BitmapImage( stream_buffer& image_data, const std::string& image_format );

		~BitmapImage() { Release(); }

		inline bool LoadFromFile( const std::string& pathname, int flag = 0 );

		inline bool SaveToFile( const std::string& pathname, int flag = 0 );

		inline bool CreateFromImageDataStream( stream_buffer& image_data, const std::string& image_format );

		/**
		\brief Filll the entire image with the specified color

		All the color components shall be in the [0,255] range 
		*/
		inline void FillColor( U8 r, U8 g, U8 b, U8 a );

		/**
		\brief Filll the entire image with the specified color

		All the color components shall be in the [0.0,1.0] range 
		*/
		inline void FillColor( float r, float g, float b, float a );

		/**
		\brief 'RGBAColorType' shall have the following public class members,
		each of which shall be floating point type in the range of [0.0,1.0]

		- RGBAColorType::red
		- RGBAColorType::green
		- RGBAColorType::blue
		- RGBAColorType::alpha
		*/
		template<class RGBAColorType>
		inline void FillFRGBAColor( const RGBAColorType& color );

		inline U32 GetPixelARGB32( int x, int y );

		//	inline void GetPixel( int x, int y, SFloatRGBAColor& color );

		//	inline SFloatRGBAColor GetPixel( int x, int y );

		inline void GetPixel( int x, int y, U8& r, U8& g, U8& b );

		inline void GetPixel( int x, int y, U8& r, U8& g, U8& b, U8& a );

		inline void GetPixel( int x, int y, float& r, float& g, float& b );

		inline void GetPixel( int x, int y, float& r, float& g, float& b, float& a );

		/**
		\brief Support only 24-bit/32-bit images. Float values are internally converted to U8 values.

		See FillFRGBAColor for the requirements to RGBAColorType
		*/
		template<class RGBAColorType>
		inline void SetFRGBAPixel( int x, int y, const RGBAColorType& color )
		{
			SetPixel( x, y, color.red, color.green, color.blue, color.alpha );
		}

		inline void SetPixel( int x, int y, U8 r, U8 g, U8 b );

		inline void SetPixel( int x, int y, U8 r, U8 g, U8 b, U8 a );

		inline void SetPixel( int x, int y, float r, float g, float b );

		inline void SetPixel( int x, int y, float r, float g, float b, float a );

		/// \param grayscale must be [0,255]
		inline void SetGrayscalePixel( int x, int y, U8 grayscale );

		/// \param alpha alpha component [0,255]
		inline void SetAlpha( int x, int y, U8 alpha );

		FIBITMAP *GetFBITMAP() { return m_pFreeImageBitMap; }

		inline int GetWidth() const;

		inline int GetHeight() const;

		inline bool Rescale( int dest_width, int dest_height/*, CImageFilter::Name filter */ );

		inline bool FlipVertical();

		inline shared_ptr<BitmapImage> CreateCopy() const;

		inline shared_ptr<BitmapImage> GetRescaled( int dest_width, int dest_height/*, CImageFilter::Name filter */ ) const;

		inline const char *GetColorTypeName() const;
	};


	inline void LogFreeImageError( FREE_IMAGE_FORMAT fif, const char *message )
	{
		using std::string;

		string error_message = "FreeImage: " + string(message);

		string img_format = (fif != FIF_UNKNOWN) ? FreeImage_GetFormatFromFIF(fif) : "unknown";
		error_message += " (image format: " + img_format +").";

		LOG_PRINT_ERROR( error_message );
	}


	inline void InitFreeImage()
	{
		FreeImage_SetOutputMessage(LogFreeImageError);

		LOG_PRINT( std::string("FreeImage version: ") + FreeImage_GetVersion() );
		LOG_PRINT( std::string("FreeImage copyright: ") + FreeImage_GetCopyrightMessage() );
	}


	inline BitmapImage::BitmapImage( int width, int height, int bpp )
		:
		m_BitsPerPixel(bpp)
	{
		m_pFreeImageBitMap = FreeImage_Allocate( width, height, bpp );
	}


	//inline BitmapImage::BitmapImage( int width, int height, int bpp, const RGBAColorType& color )
	//:
	//m_BitsPerPixel(bpp)
	//{
	//	m_pFreeImageBitMap = FreeImage_Allocate( width, height, bpp );
	//
	//	FillColor( color );
	//
	//}


	inline BitmapImage::BitmapImage( stream_buffer& image_data, const std::string& image_format )
		:
		m_pFreeImageBitMap(NULL),
		m_BitsPerPixel(0)
	{
		CreateFromImageDataStream( image_data, image_format );
	}



	/// only valid for bitmap of 32 bpp
	inline U32 BitmapImage::GetPixelARGB32( int x, int y )
	{
		U8 r=0, g=0, b=0, a=255;
		GetPixel( x, y, r, g, b, a );
		U32 argb32
			= (U32)a << 24
			| (U32)r << 16
			| (U32)g << 8
			| (U32)b;

		return argb32;

		/*	int bytes_per_pixel = FreeImage_GetBPP(m_pFreeImageBitMap) / sizeof(BYTE);

		if( bytes_per_pixel != 4 )
		{
		int GetPixelARGB32_does_not_work_if_bpp_is_not_32 = 1;
		return 0;
		}

		BYTE *bits = FreeImage_GetBits(m_pFreeImageBitMap);
		bits += ( y * GetWidth() + x ) * bytes_per_pixel;

		U32 argb32
		= (U32)bits[FI_RGBA_ALPHA] << 24
		| (U32)bits[FI_RGBA_RED]   << 16
		| (U32)bits[FI_RGBA_GREEN] << 8
		| (U32)bits[FI_RGBA_BLUE];

		return argb32;*/
	}


	//inline void BitmapImage::GetPixel( int x, int y, SFloatRGBAColor& color )
	//{
	//	U8 r=0, g=0, b=0, a=0;
	//	GetPixel( x, y, r, g, b, a );
	//	color.red   = (float)r / (float)255.0f;
	//	color.green = (float)g / (float)255.0f;
	//	color.blue  = (float)b / (float)255.0f;
	//	color.alpha = (float)a / (float)255.0f;
	//}


	//inline SFloatRGBAColor BitmapImage::GetPixel( int x, int y )
	//{
	//	SFloatRGBAColor dest;
	//	GetPixel( x, y, dest );
	//	return dest;
	//}


	inline void BitmapImage::GetPixel( int x, int y, float& r, float& g, float& b )
	{
		U8 _r=0, _g=0, _b=0;
		GetPixel( x, y, _r, _g, _b );
		r = (float)_r / (float)255.0f;
		g = (float)_g / (float)255.0f;
		b = (float)_b / (float)255.0f;
	}


	inline void BitmapImage::GetPixel( int x, int y, float& r, float& g, float& b, float& a )
	{
		U8 _r=0, _g=0, _b=0, _a=0;
		GetPixel( x, y, _r, _g, _b, _a );
		r = (float)_r / (float)255.0f;
		g = (float)_g / (float)255.0f;
		b = (float)_b / (float)255.0f;
		a = (float)_a / (float)255.0f;
	}


	inline void BitmapImage::GetPixel( int x, int y, U8& r, U8& g, U8& b )
	{
		RGBQUAD quad;
		memset( &quad, 0, sizeof(RGBQUAD) );

		FreeImage_GetPixelColor( m_pFreeImageBitMap, x, GetHeight() - y - 1, &quad );
		r = quad.rgbRed;
		g = quad.rgbGreen;
		b = quad.rgbBlue;
	}


	inline void BitmapImage::GetPixel( int x, int y, U8& r, U8& g, U8& b, U8& a )
	{
		BYTE *bits = FreeImage_GetScanLine(m_pFreeImageBitMap, GetHeight() - y - 1) + m_BitsPerPixel / 8 * x;

		r = bits[FI_RGBA_RED];
		g = bits[FI_RGBA_GREEN];
		b = bits[FI_RGBA_BLUE];

		if( m_BitsPerPixel == 32 )
			a = bits[FI_RGBA_ALPHA];
	}


	inline void BitmapImage::FillColor( U8 r, U8 g, U8 b, U8 a )
	{
		const int w = GetWidth();
		const int h = GetHeight();
		const int bytespp = FreeImage_GetLine(m_pFreeImageBitMap) / FreeImage_GetWidth(m_pFreeImageBitMap);
		for( int y = 0; y < h; y++)
		{
			BYTE *bits = FreeImage_GetScanLine(m_pFreeImageBitMap, y);
			for( int x = 0; x < w; x++)
			{
				// Set pixel color
				bits[FI_RGBA_RED]   = r;
				bits[FI_RGBA_GREEN] = g;
				bits[FI_RGBA_BLUE]  = b;
				bits[FI_RGBA_ALPHA] = a;

				// jump to next pixel
				bits += bytespp;
			}
		}
	}


	template<class RGBAColorType>
	inline void BitmapImage::FillFRGBAColor( const RGBAColorType& color )
	{
		if( !m_pFreeImageBitMap )
			return;

		U8 r = (U8)get_clamped( (int)(color.red   * 255), 0, 255 );
		U8 g = (U8)get_clamped( (int)(color.green * 255), 0, 255 );
		U8 b = (U8)get_clamped( (int)(color.blue  * 255), 0, 255 );
		U8 a = (U8)get_clamped( (int)(color.alpha * 255), 0, 255 );

		FillColor(r,g,b,a);
	}


	inline void BitmapImage::SetPixel( int x, int y, U8 r, U8 g, U8 b )
	{
		RGBQUAD quad;
		quad.rgbRed   = r;
		quad.rgbGreen = g;
		quad.rgbBlue  = b;

		FreeImage_SetPixelColor( m_pFreeImageBitMap, x, GetHeight() - y - 1, &quad );
	}


	inline void BitmapImage::SetPixel( int x, int y, U8 r, U8 g, U8 b, U8 a )
	{
		if( !m_pFreeImageBitMap )
			return;

		if( m_BitsPerPixel == 24 )
		{
			SetPixel( x, y, r, g, b );
		}
		else if( m_BitsPerPixel == 32 )
		{
			// This code causes crash when the bpp is 32. Why?

			BYTE *bits = FreeImage_GetScanLine(m_pFreeImageBitMap, GetHeight() - y - 1) + m_BitsPerPixel / 8 * x;

			int bytespp = FreeImage_GetLine(m_pFreeImageBitMap) / FreeImage_GetWidth(m_pFreeImageBitMap);

			bits[FI_RGBA_RED]   = r;
			bits[FI_RGBA_GREEN] = g;
			bits[FI_RGBA_BLUE]  = b;
			bits[FI_RGBA_ALPHA] = a;
		}
	}


	void BitmapImage::SetPixel( int x, int y, float r, float g, float b )
	{
		SetPixel(
			x, y,
			(U8)(get_clamped(r,0.0f,1.0f) * 255.0f),
			(U8)(get_clamped(g,0.0f,1.0f) * 255.0f),
			(U8)(get_clamped(b,0.0f,1.0f) * 255.0f)
		);
	}


	void BitmapImage::SetPixel( int x, int y, float r, float g, float b, float a )
	{
		SetPixel(
			x, y,
			(U8)(get_clamped(r,0.0f,1.0f) * 255.0f),
			(U8)(get_clamped(g,0.0f,1.0f) * 255.0f),
			(U8)(get_clamped(b,0.0f,1.0f) * 255.0f),
			(U8)(get_clamped(a,0.0f,1.0f) * 255.0f)
		);
	}


	inline void BitmapImage::SetGrayscalePixel( int x, int y, U8 grayscale )
	{
		RGBQUAD quad;
		quad.rgbRed   = grayscale;
		quad.rgbGreen = grayscale;
		quad.rgbBlue  = grayscale;

		//	FreeImage_SetPixelColor( m_pFreeImageBitMap, x, y, &quad );
		FreeImage_SetPixelColor( m_pFreeImageBitMap, x, GetHeight() - y - 1, &quad );
	}


	inline void BitmapImage::SetAlpha( int x, int y, U8 alpha )
	{
		//	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);

		BYTE *bits = FreeImage_GetScanLine(m_pFreeImageBitMap, GetHeight() - y - 1) + m_BitsPerPixel / 8 * x;

		bits[FI_RGBA_ALPHA] = alpha;
	}


	inline int BitmapImage::GetWidth() const
	{
		if( m_pFreeImageBitMap )
			return (int)FreeImage_GetWidth(m_pFreeImageBitMap);
		else
			return 0;
	}


	inline int BitmapImage::GetHeight() const
	{
		if( m_pFreeImageBitMap )
			return (int)FreeImage_GetHeight(m_pFreeImageBitMap);
		else
			return 0;
	}


	inline bool BitmapImage::Rescale( int dest_width, int dest_height/*, CImageFilter::Name filter */ )
	{
		if( m_pFreeImageBitMap )
		{
			// FreeImage_Rescale() does not change the image specified as the first argument.
			// It returns the scaled image.
			FIBITMAP *pScaledImage = FreeImage_Rescale( m_pFreeImageBitMap, dest_width, dest_height, FILTER_BILINEAR/*filter*/ );
			Reset( pScaledImage );
			return true;
		}
		else
			return false;
	}


	inline bool BitmapImage::FlipVertical()
	{
		if( m_pFreeImageBitMap )
		{
			BOOL res = FreeImage_FlipVertical( m_pFreeImageBitMap );
			return res ? true : false;
		}
		else
			return false;
	}


	inline shared_ptr<BitmapImage> BitmapImage::CreateCopy() const
	{
		if( !m_pFreeImageBitMap )
		{
			LOG_PRINT_ERROR( " The source bitmap image is not a valid." );
			return shared_ptr<BitmapImage>();
		}

		const int w = GetWidth();
		const int h = GetHeight();

		if( w <= 0 || h <= 0 )
		{
			LOG_PRINT_ERROR( " The source bitmap image does not have a width/height." );
			return shared_ptr<BitmapImage>();
		}

		// FreeImage_Rescale() does not change the image specified as the first argument;
		// it returns the scaled image.
		// Also note that the right and bottom argument values are set to width and height.
		// The FreeImage documentation says the arguments are LTRB, but it also says
		// "the returned bitmap is defined by a width equal to (right - left)
		// and a height equal to (bottom - top)", and also through the experimentation
		// it was found that the last two args do actually have to be set to W and H,
		// so we stick with W and H.
		FIBITMAP *pCopiedBitMap = FreeImage_Copy( m_pFreeImageBitMap, 0, 0, w, h );
		if( !pCopiedBitMap )
		{
			LOG_PRINT_ERROR( " FreeImage_Copy() returned NULL." );
			return shared_ptr<BitmapImage>();
		}

		shared_ptr<BitmapImage> pScaledImage( new BitmapImage( pCopiedBitMap ) );
		return pScaledImage;
	}


	inline shared_ptr<BitmapImage> BitmapImage::GetRescaled( int dest_width, int dest_height/*, CImageFilter::Name filter */ ) const
	{
		if( m_pFreeImageBitMap )
		{
			// FreeImage_Rescale() does not change the image specified as the first argument.
			// It returns the scaled image.
			FIBITMAP *pScaledBitMap = FreeImage_Rescale( m_pFreeImageBitMap, dest_width, dest_height, FILTER_BILINEAR/*filter*/ );
			shared_ptr<BitmapImage> pScaledImage( new BitmapImage( pScaledBitMap ) );
			return pScaledImage;
		}
		else
			return shared_ptr<BitmapImage>();
	}


	inline const char *BitmapImage::GetColorTypeName() const
	{
		if( !m_pFreeImageBitMap )
			return "(image is not loaded)";

		FREE_IMAGE_COLOR_TYPE color_type = FreeImage_GetColorType( m_pFreeImageBitMap );

		switch( color_type )
		{
		case FIC_MINISWHITE: return "MINISWHITE";
		case FIC_MINISBLACK: return "MINISBLACK";
		case FIC_RGB:        return "RGB";
		case FIC_PALETTE:    return "PALETTE";
		case FIC_RGBALPHA:   return "RGBA";
		case FIC_CMYK:       return "CMYK";
		default:
			return "(an unknown color type)";
		}
	}


	// ----------------------------------------------------------

	/**
	@brief Loads an image from a file

	@param pathname Pointer to the full file name
	@param flag Optional load flag constant

	@return Returns true on success
	*/
	inline bool BitmapImage::LoadFromFile( const std::string& pathname, int flag )
	{
		Release();

		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

		// check the file signature and deduce its format
		// (the second argument is currently not used by FreeImage)
		fif = FreeImage_GetFileType( pathname.c_str(), 0 );
		if(fif == FIF_UNKNOWN)
		{
			// no signature ?
			// try to guess the file format from the file extension
			fif = FreeImage_GetFIFFromFilename( pathname.c_str() );
		}

		if(fif == FIF_UNKNOWN)
		{
			LOG_PRINT_ERROR( " An unsupported image file format: " + pathname );
			return false;
		}

		// check that the plugin has reading capabilities ...
		if( !FreeImage_FIFSupportsReading(fif) )
		{
			LOG_PRINT_ERROR( " FreeImage_FIFSupportsReading() failed on the following image file: " + pathname );
			return false;
		}

		// ok, let's load the file
		m_pFreeImageBitMap = FreeImage_Load(fif, pathname.c_str(), flag);

		// unless a bad file format, we are done !
		if( m_pFreeImageBitMap )
		{
			m_BitsPerPixel = FreeImage_GetBPP( m_pFreeImageBitMap );
			return true;
		}
		else
		{
			LOG_PRINT_ERROR( " Failed to load a file: " + pathname );
			return false;
		}
	}


	inline bool BitmapImage::Save32BPPImageTo24JPEGFile( const std::string& image_file_pathname, int flag )
	{
		int width  = this->GetWidth();
		int height = this->GetHeight();
		int bpp    = this->m_BitsPerPixel;

		if( 4 <= image_file_pathname.length()
			&& image_file_pathname.find(".jpg") == image_file_pathname.length() - 4
			&& bpp == 32 )
		{
			BitmapImage img_bpp24( width, height, 24 );
			if( !img_bpp24.m_pFreeImageBitMap )
				return false;

			for( int y=0; y<height; y++ )
			{
				for( int x=0; x<width; x++ )
				{
					U8 r=0,g=0,b=0;
					GetPixel(x,y,r,g,b);
					img_bpp24.SetPixel(x,y,r,g,b);
				}
			}

			BOOL succeeded = FreeImage_Save(FIF_JPEG, img_bpp24.m_pFreeImageBitMap, image_file_pathname.c_str(), flag);
			return succeeded ? true : false;
		}

		return false;
	}

	/** 
	@brief Save the bitmap to a file

	@param pathname Pointer to the full file name
	@param flag Optional save flag constant

	@return Returns true if successful, returns false otherwise
	*/
	inline bool BitmapImage::SaveToFile( const std::string& pathname, int flag )
	{
		if( !m_pFreeImageBitMap )
		{
			LOG_PRINT_ERROR( " Has no valid bitmap." );
			return false;
		}

		// try to guess the file format from the file extension
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename( pathname.c_str() );
		if(fif == FIF_UNKNOWN )
		{
			LOG_PRINT_ERROR( " An unsupported image format: " + pathname );
			return false;
		}

		// check that the plugin has sufficient writing and export capabilities ...
		WORD bpp = FreeImage_GetBPP(m_pFreeImageBitMap);
		BOOL supports_writing = FreeImage_FIFSupportsWriting(fif);
		if( !supports_writing )
		{
			LOG_PRINT_ERROR( " FreeImage_FIFSupportsWriting() returned false. Cannot save the image to the file as " + pathname );
			return false;
		}

		if( 4 <= pathname.length()
			&& pathname.find(".jpg") == pathname.length() - 4
			&& bpp == 32 )
		{
			// 32-bit (RGBA) images cannot be saved to JPEG files.
			// Give up the alpha and save the bitmap as a 24-bit (RGB) image file.
			LOG_PRINT_WARNING( " 32-bit images cannot be saved to JPEG files. Saving the image as 24-bit bitmap without the alpha channel: " + pathname );
			return Save32BPPImageTo24JPEGFile(pathname,flag);
		}

		BOOL supports_export_bpp = FreeImage_FIFSupportsExportBPP(fif, bpp);
		if( !supports_export_bpp )
		{
			LOG_PRINT_ERROR( " Cannot save an image to the file: " + pathname );
			return false;
		}

		// ok, we can save the file
		BOOL succeeded = FreeImage_Save(fif, m_pFreeImageBitMap, pathname.c_str(), flag);

		// unless an abnormal bug, we are done !
		if( succeeded == TRUE )
		{
			LOG_PRINT_VERBOSE( " Saved an image to the file: " + pathname );
			return true;
		}
		else
		{
			LOG_PRINT_ERROR( " Failed to save an image to the file: " + pathname );
			return false;
		}
	}


	inline bool BitmapImage::CreateFromImageDataStream( stream_buffer& image_data, const std::string& image_format )
	{

		//	lock

		GetImageStreamBufferHolder().m_pStreamBuffer = &image_data;//(img_archive.m_Buffer);
		FreeImageIO img_io;
		img_io.read_proc  = ImageReadProc;
		img_io.write_proc = ImageWriteProc;
		img_io.seek_proc  = ImageSeekProc;
		img_io.tell_proc  = ImageTellProc;

		int sth = 0;

		int flags = 0;
		m_pFreeImageBitMap = FreeImage_LoadFromHandle( ToFIF(image_format), &img_io, &sth, flags );

		if( m_pFreeImageBitMap )
		{
			m_BitsPerPixel = FreeImage_GetBPP( m_pFreeImageBitMap );
			return true;
		}
		else
		{
			LOG_PRINT_ERROR( "FreeImage_LoadFromHandle() returned null. Image format: " + image_format );
			return false;
		}
	}

	//
	// Global Functions
	//

	inline shared_ptr<BitmapImage> CreateBitmapImage( const std::string& pathname, int flag = 0 )
	{
		shared_ptr<BitmapImage> pImage( new BitmapImage() );

		bool bSuccess = pImage->LoadFromFile( pathname, flag );

		return bSuccess ? pImage : shared_ptr<BitmapImage>();
	}


} // namespace amorphous


#endif /* __amorphous_BitmapImage_FreeImage_HPP__ */
