#ifndef Z_FILL_STYLE_H
#define Z_FILL_STYLE_H

#include "swf/render_handler.h"
#include "rgba.h"
#include "matrix.h"

class MovieDefinition;
class Stream;
class BitmapCharacterDef;

struct GradientRecord
{
	GradientRecord();
	void	read(Stream* in, int tag_type);

	//data:
	UInt8	_ratio;
	RGBA	_color;
};

class BaseFillStyle
{
public:
	virtual ~BaseFillStyle() {}
	virtual void apply(int fillSide, float ratio, BitmapBlendMode bm) const = 0;
};

class FillStyle : public BaseFillStyle
{
public:
	FillStyle();
	virtual ~FillStyle();

	void	read(Stream* in, int tagType, MovieDefinition* m);
	void	apply(int fillSide, float ratio, BitmapBlendMode bm) const;
	BitmapInfo*					createGradientBitmap() const;
	RGBA						sampleGradient(int ratio) const;
private:
	int							_type;
	RGBA						_color;
	Matrix						_gradientMatrix;
	std::vector<GradientRecord>	_gradients;
	Matrix						_bitmapMatrix;
	BitmapCharacterDef*			_bitmapChDef;
	BitmapInfo*					_gradientBitmapInfo;
};
#endif