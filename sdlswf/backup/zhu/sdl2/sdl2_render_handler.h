#ifndef Z_SDL2_RENDER_HANDLER_H
#define Z_SDL2_RENDER_HANDLER_H

#include "swf/render_handler.h"
#include "swf/type/cxform.h"
#include "swf/type/matrix.h"
#include "swf/type/rgba.h"

#include "SDL.h"

#include "base/image.h"

namespace RenderType
{
	struct FillStyle
	{
		enum mode
		{
			INVALID,
			COLOR,
			BITMAP_WRAP,
			BITMAP_CLAMP,
			LINEAR_GRADIENT,
			RADIAL_GRADIENT,
		};
		mode			_mode;
		RGBA			_color;
		BitmapInfo*		_bitmapInfo;
		Matrix			_bitmapMatrix;
		CXForm			_bitmapColorTransform;
		bool			_hasNonZeroBitmapAdditiveColor;
		float			_width;	// for line style

		FillStyle() :
			_mode(INVALID),
			_hasNonZeroBitmapAdditiveColor(false)
		{
		}

		void	apply(/*const matrix& current_matrix*/) const;
			// Push our style into OpenGL.


		void	disable() { _mode = INVALID; }
		void	setColor(RGBA color) { _mode = COLOR; _color = color; }
		void	setBitmap(BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, const CXForm& color_transform);
		bool	isValid() const { return _mode != INVALID; }
	};


	// Style state.
	enum style_index
	{
		LEFT_STYLE = 0,
		RIGHT_STYLE,
		LINE_STYLE,

		STYLE_COUNT
	};
};


class SDL2RenderHandler : public RenderHandler
{
public:	
	SDL2RenderHandler(SDL_Renderer* r);
	~SDL2RenderHandler();
	static	SDL2RenderHandler* getInst();

	virtual void	begin(RGBA bgc, int viewport_x0, int viewport_y0, int viewport_width, int viewport_height,
							float x0, float x1, float y0, float y1);
	virtual void	end();
	virtual void	drawBitmap(const Matrix& m, BitmapInfo* bi, const RECT& coords, const RECT& uv_coords, RGBA color);
	virtual void	fillStyleColor(int fill_side, const RGBA& color);
	virtual void	fillStyleBitmap(int fill_side, BitmapInfo* bi, const Matrix& m, BitmapWrapMode wm, BitmapBlendMode bm);

	virtual void	lineStyleColor(RGBA color);
	virtual void	lineStyleWidth(float width);

	virtual BitmapInfo*	createBitmapInfoEmpty();
	virtual BitmapInfo*	createBitmapInfoAlpha(int w, int h, unsigned char* data);
	virtual BitmapInfo*	createBitmapInfoRGB(Image::RGB* im);
	virtual BitmapInfo*	createBitmapInfoRGBA(Image::RGBA* im);

			void	applyColor(const RGBA& c);
		// Set the given color.
private:
	SDL_Renderer*	_render;

	Matrix			_currentMatrix;
	CXForm			_currentCXForm;

	static SDL2RenderHandler*	_inst;

	FillStyle					_currentStyles[STYLE_COUNT];
};
#endif