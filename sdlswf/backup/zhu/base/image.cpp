#include "image.h"
#include "global.h"

using namespace Image;

ImageBase::ImageBase(UInt8* data, int width, int height, int pitch, IdImage type)
:_type(type),
_data(data),
_width(width),
_height(height),
_pitch(pitch)
{

}

ImageBase::~ImageBase()
{
	if (_data)
	{
		delete[] _data;
		_data = NULL;
	}
}

RGB::RGB(int width, int height) :ImageBase(0, width, height, (width * 3 + 3) & ~3, E_RGB)
// round pitch up to nearest 4-byte boundary
{
	Assert(width > 0);
	Assert(height > 0);
	Assert(_pitch >= _width * 3);
	Assert((_pitch & 3) == 0);

	//		m_data = (Uint8*) dlmalloc(m_pitch * m_height);
	_data = new UInt8[_pitch * _height];
}

RGB::~RGB()
{
	if (_data) 
	{		
		delete [] _data;
		_data = NULL;
	}
}

RGBA::RGBA(int width, int height) :ImageBase(0, width, height, width * 4, E_RGBA)
{
	Assert(width > 0);
	Assert(height > 0);
	Assert(_pitch >= _width * 4);
	Assert((_pitch & 3) == 0);

	_data = new UInt8[_pitch * _height];
}

RGBA::~RGBA()
{
	if (_data) 
	{
		delete[] _data;
		_data = 0;
	}
}

void RGBA::setPixel(int x, int y, UInt8 r, UInt8 g, UInt8 b, UInt8 a)
{
	Assert(x >= 0 && x < _width);
	Assert(y >= 0 && y < _height);

	UInt8*	data = scanline(this, y) + 4 * x;

	data[0] = r;
	data[1] = g;
	data[2] = b;
	data[3] = a;
}

UInt8*	scanline(ImageBase* surf, int y)
{
	Assert(surf);
	Assert(y >= 0 && y < surf->_height);
	return ((UInt8*)surf->_data) + surf->_pitch * y;
}

const UInt8*	scanline(const ImageBase* surf, int y)
{
	Assert(surf);
	Assert(y >= 0 && y < surf->_height);
	return ((const UInt8*)surf->_data) + surf->_pitch * y;
}

// Make a system-memory 24-bit bitmap surface.  24-bit packed
// data, red byte first.
RGB* createRGB(int width, int height)
{
	RGB* s = new RGB(width, height);

	return s;
}

// Make a system-memory 32-bit bitmap surface.  Packed data,
// red byte first.
RGBA* createRGBA(int width, int height)
{
	RGBA*	s = new RGBA(width, height);

	return s;
}

// Make a system-memory 8-bit bitmap surface.
Alpha* createAlpha(int width, int height)
{
	Alpha*	s = new Alpha(width, height);

	return s;
}

Alpha::Alpha(int width, int height) :ImageBase(0, width, height, width, E_ALPHA)
{
	Assert(width > 0);
	Assert(height > 0);

	//		m_data = (Uint8*) dlmalloc(m_pitch * m_height);
	_data = new UInt8[_pitch * _height];
}

Alpha::~Alpha()
{
	if (_data) 
	{
		delete[] _data;
		_data = NULL;
	}
}

void Alpha::setPixel(int x, int y, UInt8 a)
{
	Assert(x >= 0 && x < _width);
	Assert(y >= 0 && y < _height);

	UInt8*	data = scanline(this, y) + x;

	data[0] = a;
}

// Bitwise content comparison.
bool Alpha::operator==(const Alpha& a) const
{
	if (_width != a._width
		|| _height != a._height)
	{
		return false;
	}

	for (int j = 0, n = _height; j < n; j++)
	{
		if (memcmp(scanline(this, j), scanline(&a, j), _width))
		{
			// Mismatch.
			return false;
		}
	}

	// Images are identical.
	return true;
}

// Return a hash code based on the image contents.
unsigned int Alpha::computeHash() const
{
	unsigned int	h = (unsigned int)Base::bernsteinHash(&_width, sizeof(_width));
	h = (unsigned int)Base::bernsteinHash(&_height, sizeof(_height), h);

	for (int i = 0, n = _height; i < n; i++)
	{
		h = (unsigned int)Base::bernsteinHash(scanline(this, i), _width, h);
	}

	return h;
}
