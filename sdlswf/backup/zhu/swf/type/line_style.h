#ifndef Z_LINE_STYLE_H
#define Z_LINE_STYLE_H

#include "global.h"
#include "rgba.h"
#include "fill_style.h"

class Stream;
class MovieDefinition;

class BaseLineStyle
{
public:
	virtual ~BaseLineStyle() {}
	virtual void apply(float ratio) const = 0;
};

class LineStyle : public BaseLineStyle
	// For the outside of outline shapes, or just bare lines.
{
public:
	LineStyle();
	virtual ~LineStyle() {}

	void	read(Stream* in, int tagType, MovieDefinition* m);
	void	apply(float ratio) const;

private:
	UInt16			_width;	// in TWIPS
	RGBA			_color;
	FillStyle		_fillStyle;
	UInt8			_startCapstyle;
	UInt8			_joinstyle;
	bool			_hasFillFlag;
	bool			_noHScaleFlag;
	bool			_noVScaleFlag;
	bool			_pixelHintingFlag;
	bool			_noClose;
	UInt8			_endCapstyle;
	UInt16			_miterLimitFactor;
};
#endif