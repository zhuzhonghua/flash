#include "line_style.h"

LineStyle::LineStyle() :
_width(0),
_startCapstyle(0),
_joinstyle(0),
_hasFillFlag(false),
_noHScaleFlag(false),
_noVScaleFlag(false),
_pixelHintingFlag(false),
_noClose(false),
_endCapstyle(0),
_miterLimitFactor(0)
{

}

void LineStyle::read(Stream* in, int tagType, MovieDefinition* m)
{
	_width = in->readUI16();
	if (tagType == 83)	// SHAPE 4
	{
		// 0 = Round cap
		// 1 = No cap
		// 2 = Square cap
		_startCapstyle = in->readUInt(2);

		// 0 = Round join
		// 1 = Bevel join
		// 2 = Miter join
		_joinstyle = in->readUInt(2);

		// If 0, uses Color field.
		_hasFillFlag = in->readUInt(1) == 1 ? true : false;

		// If 1, stroke thickness will not scale if the object is scaled horizontally.
		_noHScaleFlag = in->readUInt(1) == 1 ? true : false;

		// If 1, stroke thickness will not scale if the object is scaled vertically.
		_noVScaleFlag = in->readUInt(1) == 1 ? true : false;

		// If 1, all anchors will be aligned to full pixels.
		_pixelHintingFlag = in->readUInt(1) == 1 ? true : false;

		in->readUInt(5); // Reserved UB[5] Must be 0.

		// If 1, stroke will not be closed if the stroke’s last point
		// matches its first point. Flash Player will apply caps instead of a join.
		_noClose = in->readUInt(1) == 1 ? true : false;

		// 0 = Round cap
		// 1 = No cap
		// 2 = Square cap
		_endCapstyle = in->readUInt(2);

		if (_joinstyle == 2)
		{
			// Miter limit factor is an 8.8 fixed-point value
			_miterLimitFactor = in->readUI16();
		}

		if (_hasFillFlag)
		{
			_fillStyle.read(in, tagType, m);
		}
		else
		{
			_color.read(in, tagType);
		}
	}
	else
	{
		_color.read(in, tagType);
	}
}

void LineStyle::apply(float ratio) const
{
	Render::line_style_color(_color);
	Render::line_style_width(_width);
}