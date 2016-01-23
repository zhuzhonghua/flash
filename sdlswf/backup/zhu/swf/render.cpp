#include "render.h"
#include "system.h"
#include "render_handler.h"
#include "base/image.h"

namespace Render
{
	using namespace System;
	void	beginDisplay(RGBA backgroundColor,
		int viewport_x0, int viewport_y0, int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1)
	{
		RenderHandler* r = Config::get()->getRenderHandler();

		r->begin(backgroundColor, viewport_x0, viewport_y0, viewport_width, viewport_height, x0, x1, y0, y1);
	}

	void	endDisplay()
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->end();
	}

	void	drawBitmap(const Matrix& m, BitmapInfo* bi, const RECT& coords, const RECT& uv_coords, RGBA color)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->drawBitmap(m,bi,coords,uv_coords,color);
	}

	void	fillStyleColor(int fill_side, const RGBA& color)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->fillStyleColor(fill_side, color);
	}
	void	fillStyleBitmap(int fill_side, BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, BitmapBlendMode bm)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->fillStyleBitmap(fill_side, bi, m, wm, bm);
	}
	void	lineStyleColor(RGBA color)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->lineStyleColor(color);
	}
	void	lineStyleWidth(float width)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->lineStyleWidth(width);

	}
	BitmapInfo*	createBitmapInfoEmpty()
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		return r->createBitmapInfoEmpty();
	}

	BitmapInfo*	createBitmapInfoAlpha(int w, int h, unsigned char* data)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->createBitmapInfoAlpha(w, h, data);
	}

	BitmapInfo*	createBitmapInfoRGB(Image::RGB* im)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->createBitmapInfoRGB(im);
	}

	BitmapInfo*	createBitmapInfoRGBA(Image::RGBA* im)
	{
		RenderHandler* r = Config::get()->getRenderHandler();
		r->createBitmapInfoRGBA(im);
	}
}