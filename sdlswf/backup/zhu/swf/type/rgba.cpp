#include "rgba.h"

RGBA::RGBA() : 
_r(255), 
_g(255), 
_b(255), 
_a(255) 
{

}

RGBA::RGBA(UInt8 r, UInt8 g, UInt8 b, UInt8 a) :
_r(r), 
_g(g), 
_b(b), 
_a(a)
{
}

RGBA::RGBA(double color)
{
	set(color);
}

void RGBA::read(Stream* in, const TagInfo& info)
{
	if (info.tagType <= SWF::DEFINESHAPE2)
	{
		readRGB(in);
	}
	else
	{
		readRGBA(in);
	}
}

void RGBA::read(Stream* in, int tag_type)
{
	TagInfo ti;
	ti.tagType = (SWF::Tag)tag_type;
	read(in, ti);
}

void RGBA::readRGBA(Stream* in)
{
	readRGB(in);
	_a = in->readUI8();
}

void RGBA::readRGB(Stream* in)
{
	_r = in->readUI8();
	_g = in->readUI8();
	_b = in->readUI8();

	_a = 0xFF;
}

void RGBA::set(UInt8 r, UInt8 g, UInt8 b, UInt8 a)
{
	_r = r;
	_g = g;
	_b = b;
	_a = a;
}

void RGBA::set(double color)
{
	int rgb = int(color);
	_r = (UInt8)(rgb >> 16) & 0xFF;
	_g = (UInt8)(rgb >> 8) & 0xFF;
	_b = (UInt8)rgb & 0xFF;
	_a = 0xFF;
}

void RGBA::setLerp(const RGBA& a, const RGBA& b, float f)
{
	_r = (UInt8)Base::frnd(Base::flerp(a._r, b._r, f));
	_g = (UInt8)Base::frnd(Base::flerp(a._g, b._g, f));
	_b = (UInt8)Base::frnd(Base::flerp(a._b, b._b, f));
	_a = (UInt8)Base::frnd(Base::flerp(a._a, b._a, f));
}