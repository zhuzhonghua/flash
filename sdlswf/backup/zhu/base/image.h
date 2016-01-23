#ifndef Z_IMAGE_H
#define Z_IMAGE_H

#include "types.h"

int p2(int n);


namespace Image
{
	struct ImageBase
	{
		enum IdImage
		{
			E_INVALID,
			E_RGB,
			E_RGBA,
			E_ALPHA
		};

		IdImage		_type;
		UInt8*		_data;
		int			_width;
		int			_height;
		int			_pitch;	// byte offset from one row to the next

		ImageBase(UInt8* data, int width, int height, int pitch, IdImage type);
		virtual ~ImageBase();
	};

	// 24-bit RGB image.  Packed data, red byte first (RGBRGB...)
	struct RGB : public ImageBase
	{
		RGB(int width, int height);
		~RGB();
	};

	// 32-bit RGBA image.  Packed data, red byte first (RGBARGBA...)
	struct RGBA : public ImageBase
	{
		RGBA(int width, int height);
		~RGBA();

		void	setPixel(int x, int y, UInt8 r, UInt8 g, UInt8 b, UInt8 a);
	};

	// 8-bit alpha image.
	struct Alpha : public ImageBase
	{
		Alpha(int width, int height);
		~Alpha();

		void	setPixel(int x, int y, UInt8 a);

		// Bitwise content comparison.
		bool	operator==(const Alpha& a) const;

		// Return a hash code based on the image contents.
		unsigned int	computeHash() const;
	};


	// Make a system-memory 24-bit bitmap surface.  24-bit packed
	// data, red byte first.
	RGB*	createRGB(int width, int height);

	// Make a system-memory 32-bit bitmap surface.  Packed data,
	// red byte first.
	RGBA*	createRGBA(int width, int height);

	// Make a system-memory 8-bit bitmap surface.
	Alpha*	createAlpha(int width, int height);

	UInt8*	scanline(ImageBase* surf, int y);
	const UInt8*	scanline(const ImageBase* surf, int y);
};

#endif