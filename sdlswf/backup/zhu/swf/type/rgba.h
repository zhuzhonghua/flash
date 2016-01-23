#ifndef Z_RGBA_
#define Z_RGBA_

#include "types.h"
#include "swf/stream.h"

class RGBA
{
public:
	RGBA();
	RGBA(UInt8 r, UInt8 g, UInt8 b, UInt8 a);

	RGBA(double color);
	void	read(Stream* in, const TagInfo &info);
	void	read(Stream* in, int tag_type);
	void	readRGBA(Stream* in);
	void	readRGB(Stream* in);

	void	set(UInt8 r, UInt8 g, UInt8 b, UInt8 a);
	
	UInt8	getR() { return _r; }
	UInt8	getG() { return _g; }
	UInt8	getB() { return _b; }

	void	set(double color);

	void	setA(UInt8 a) { _a = a; };
	UInt8	getA() const { return _a; }

	void	setLerp(const RGBA& a, const RGBA& b, float f);

public:
	UInt8	_r, _g, _b, _a;
};
#endif