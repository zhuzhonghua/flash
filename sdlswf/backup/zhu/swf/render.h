#ifndef Z_RENDER_H
#define Z_RENDER_H

#include "type/rgba.h"
#include "type/matrix.h"
#include "type/rect.h"
#include "base/image.h"
#include "render_handler.h"

class BitmapInfo;

namespace Render
{
	BitmapInfo*	createBitmapInfoEmpty();
	BitmapInfo*	createBitmapInfoAlpha(int w, int h, unsigned char* data);
	BitmapInfo*	createBitmapInfoRGB(Image::RGB* im);
	BitmapInfo*	createBitmapInfoRGBA(Image::RGBA* im);

	void	beginDisplay(RGBA backgroundColor,
		int viewport_x0, int viewport_y0, int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1);
	void	endDisplay();

	void	drawBitmap(const Matrix& m, BitmapInfo* bi, const RECT& coords, const RECT& uv_coords, RGBA color);

	void	fillStyleColor(int fill_side, const RGBA& color);
	void	fillStyleBitmap(int fill_side, BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, BitmapBlendMode bm);

	void	lineStyleColor(RGBA color);
	void	lineStyleWidth(float width);
}
#endif