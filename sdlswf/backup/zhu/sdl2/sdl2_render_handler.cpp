#include "sdl2_render_handler.h"
#include "swf/type/point.h"
#include "swf/type/matrix.h"
#include "swf/type/bitmap_info.h"

class BitmapInfoSDL2 : public BitmapInfo
{
public:
	int _width;
	int _height;
	Image::ImageBase* _suspended_image;

	BitmapInfoSDL2();
	BitmapInfoSDL2(int width, int height, UInt8* data);
	BitmapInfoSDL2(Image::RGB* im);
	BitmapInfoSDL2(Image::RGBA* im);

	virtual void layout();

	// get byte per pixel
	virtual int getBPP() const
	{
		if (_suspended_image)
		{
			switch (_suspended_image->_type)
			{
			default: return 0;
			case Image::ImageBase::E_RGB: return 3;
			case Image::ImageBase::E_RGBA: return 4;
			case Image::ImageBase::E_ALPHA: return 1;
			};
		}
		return 0;
	}

	virtual unsigned char* getData() const
	{
		if (_suspended_image)
		{
			return _suspended_image->_data;
		}
		return NULL;
	}

	virtual void activate()
	{
		// TODO
	}

	~BitmapInfoSDL2()
	{
		delete _suspended_image;
	}

	virtual int getWidth() const { return _width; }
	virtual int getHeight() const { return _height; }

};

BitmapInfoSDL2::BitmapInfoSDL2() :
_width(0),
_height(0),
_suspended_image(0)
{

}

BitmapInfoSDL2::BitmapInfoSDL2(int width, int height, UInt8* data) :
_width(width),
_height(height)
{
	Assert(width > 0 && height > 0 && data);
	_suspended_image = Image::createAlpha(width, height);
	memcpy(_suspended_image->_data, data, _suspended_image->_pitch * _suspended_image->_height);
}

BitmapInfoSDL2::BitmapInfoSDL2(Image::RGB* im) :
_width(im->_width),
_height(im->_height)
{
	Assert(im);
	_suspended_image = Image::createRGB(im->_width, im->_height);
	memcpy(_suspended_image->_data, im->_data, im->_pitch * im->_height);
}

BitmapInfoSDL2::BitmapInfoSDL2(Image::RGBA* im) :
_width(im->_width),
_height(im->_height)
{
	Assert(im);
	_suspended_image = Image::createRGBA(im->_width, im->_height);
	memcpy(_suspended_image->_data, im->_data, im->_pitch * im->_height);
}

void BitmapInfoSDL2::layout()
{
	// TODO
}

SDL2RenderHandler*	SDL2RenderHandler::_inst;

SDL2RenderHandler* SDL2RenderHandler::getInst()
{
	return _inst;
}

SDL2RenderHandler::SDL2RenderHandler(SDL_Renderer* r)
{
	_render = r;
	Assert(SDL2RenderHandler::_inst == NULL);
	SDL2RenderHandler::_inst = this;
}

SDL2RenderHandler::~SDL2RenderHandler()
{
	SDL2RenderHandler::_inst = NULL;
}

void SDL2RenderHandler::begin(RGBA bgc, int viewport_x0, int viewport_y0, int viewport_width, int viewport_height,
								float x0, float x1, float y0, float y1)
{
	SDL_SetRenderDrawColor(_render, bgc.getR(), bgc.getG(), bgc.getB(), bgc.getA());

	SDL_Rect rect;
	rect.x = viewport_x0; rect.y = viewport_y0; rect.w = viewport_width; rect.h = viewport_height;

	SDL_RenderFillRect(_render, &rect);
}

void SDL2RenderHandler::end()
{
	SDL_RenderPresent(_render);
}

void SDL2RenderHandler::drawBitmap(const Matrix& m, BitmapInfo* bi, const RECT& coords, const RECT& uv_coords, RGBA color)
{
	SDL_SetRenderDrawColor(_render, color.getR(), color.getG(), color.getB(), color.getA());

	Point a, b, c, d;
	m.transform(&a, Point(coords._x_min, coords._y_min));
	m.transform(&b, Point(coords._x_max, coords._y_min));
	m.transform(&c, Point(coords._x_min, coords._y_max));
	d.x = b.x + c.x - a.x;
	d.y = b.y + c.y - a.y;

	bi->layout();

	// TODO: what the fuck
	SDL_Texture* texture = SDL_CreateTexture(_render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, bi->getWidth(), bi->getHeight());
	SDL_UpdateTexture(texture, NULL, bi->getData(), bi->getWidth()*bi->getBPP());

	SDL_Rect drect;
	drect.x = (int)coords._x_min;
	drect.y = (int)coords._y_min;
	drect.w = (int)coords.width();
	drect.h = (int)coords.height();

	SDL_Rect srcrect;
	srcrect.x = (int)uv_coords._x_min;
	srcrect.y = (int)uv_coords._y_min;
	srcrect.w = (int)uv_coords.width()*bi->getWidth();
	srcrect.h = (int)uv_coords.height()*bi->getHeight();

	SDL_RenderCopy(_render, texture, &srcrect, &drect);

	SDL_DestroyTexture(texture);
}

void SDL2RenderHandler::fillStyleColor(int fill_side, const RGBA& color)
{
	Assert(fill_side >= 0 && fill_side < 2);

	_currentStyles[fill_side].set_color(_currentCXForm.transform(color));
}

void SDL2RenderHandler::fillStyleBitmap(int fill_side, BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, BitmapBlendMode bm)
{
	Assert(fill_side >= 0 && fill_side < 2);
	_currentStyles[fill_side].setBitmap(bi, m, wm, _currentCXForm);
}

void SDL2RenderHandler::lineStyleColor(RGBA color)
{
	_currentStyles[LINE_STYLE].setColor(_currentCXForm.transform(color));
}

void SDL2RenderHandler::lineStyleWidth(float width)
{
	_currentStyles[LINE_STYLE]._width = width;
}

void SDL2RenderHandler::applyColor(const RGBA& color)
// Set the given color.
{
	SDL_SetRenderDrawColor(_render, color._r, color._g, color._b, color._a);
}

BitmapInfo*	SDL2RenderHandler::createBitmapInfoEmpty()
{
	return new BitmapInfoSDL2();
}

BitmapInfo*	SDL2RenderHandler::createBitmapInfoAlpha(int w, int h, unsigned char* data)
{
	return new BitmapInfoSDL2(w, h, data);
}

BitmapInfo*	SDL2RenderHandler::createBitmapInfoRGB(Image::RGB* im)
{
	return new BitmapInfoSDL2(im);
}

BitmapInfo*	SDL2RenderHandler::createBitmapInfoRGBA(Image::RGBA* im)
{
	return new BitmapInfoSDL2(im);
}

using namespace RenderType;

void FillStyle::apply(/*const matrix& current_matrix*/) const
// Push our style into OpenGL.
{
	Assert(_mode != INVALID);

	if (_mode == COLOR)
	{
		SDL2RenderHandler::getInst()->applyColor(_color);
	}
	else if (_mode == BITMAP_WRAP || _mode == BITMAP_CLAMP)
	{
		SDL2RenderHandler::getInst()->applyColor(_color);

		_bitmapInfo->layout();

		float	inv_width = 1.0f / _bitmapInfo->getWidth();
		float	inv_height = 1.0f / _bitmapInfo->getHeight();
		// TODO
	}
}

void FillStyle::setBitmap(BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, const CXForm& color_transform)
{
	_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
	_bitmapInfo = bi;
	_bitmapMatrix = m;
	_bitmapColorTransform = color_transform;
	_bitmapColorTransform.clamp();

	_color = RGBA(
		Uint8(_bitmapColorTransform._m[0][0] * 255.0f),
		Uint8(_bitmapColorTransform._m[1][0] * 255.0f),
		Uint8(_bitmapColorTransform._m[2][0] * 255.0f),
		Uint8(_bitmapColorTransform._m[3][0] * 255.0f));

	if (_bitmapColorTransform._m[0][1] > 1.0f
		|| _bitmapColorTransform._m[1][1] > 1.0f
		|| _bitmapColorTransform._m[2][1] > 1.0f
		|| _bitmapColorTransform._m[3][1] > 1.0f)
	{
		_hasNonZeroBitmapAdditiveColor = true;
	}
	else
	{
		_hasNonZeroBitmapAdditiveColor = false;
	}
}