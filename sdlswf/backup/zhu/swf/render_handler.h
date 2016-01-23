#ifndef Z_RENDER_HANDLER_H
#define Z_RENDER_HANDLER_H

#include "type/rgba.h"
#include "type/rect.h"
#include "base/image.h"

class BitmapInfo;

// Set line and fill styles for mesh & line_strip
// rendering.
enum BitmapWrapMode
{
	WRAP_REPEAT,
	WRAP_CLAMP
};

enum BitmapBlendMode
{
	BLEND_NORMAL,
	BLEND_LAYER,
	BLEND_MULTIPLY,
	BLEND_SCREEN,
	BLEND_LIGHTEN,
	BLEND_DARKEN,
	BLEND_DIFFERENCE,
	BLEND_ADD,
	BLEND_SUBTRACT,
	BLEND_INVERT,
	BLEND_ALPHA,
	BLEND_ERASE,
	BLEND_OVERLAY,
	BLEND_HARDLIGHT
};

class RenderHandler
{
public:	
	~RenderHandler(){}

	virtual void	begin(RGBA backgroundColor, int viewport_x0, int viewport_y0, int viewport_width, int viewport_height,
							float x0, float x1, float y0, float y1) = 0;
	virtual void	end() = 0;
	virtual void	drawBitmap(const Matrix& m, BitmapInfo* bi, const RECT& coords, const RECT& uv_coords, RGBA color) = 0;
	virtual void	fillStyleColor(int fill_side, const RGBA& color) = 0;

	virtual void	fillStyleBitmap(int fill_side, BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, BitmapBlendMode bm) = 0;

	virtual void	lineStyleColor(RGBA color) = 0;
	virtual void	lineStyleWidth(float width) = 0;

	virtual BitmapInfo*	createBitmapInfoEmpty() = 0;
	virtual BitmapInfo*	createBitmapInfoAlpha(int w, int h, unsigned char* data) = 0;
	virtual BitmapInfo*	createBitmapInfoRGB(Image::RGB* im) = 0;
	virtual BitmapInfo*	createBitmapInfoRGBA(Image::RGBA* im) = 0;
};
#endif