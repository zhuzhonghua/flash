#include "stage.h"
#include "character.h"
#include "render.h"
#include "movie_definition_impl.h"

Stage::Stage(Context* ctx, MovieDefinitionImpl* def)
:_backgroundColor(0, 0, 0, 255)
, _viewportX0(0)
, _viewportY0(0)
, _viewportWidth(10)
, _viewportHeight(10)
, _pixelScale(1.0f)
{
	_ctx = ctx;
	_movieDef = def;
}

Stage::~Stage()
{

}

void Stage::setStageMovie(Character* movie)
{
	_movie = movie;
}

void Stage::advance(double dt)
{
	_movie->advance(dt);
}

void Stage::display()
{
	RECT frameSize = _movieDef->getFrameSize();
	Render::beginDisplay(_backgroundColor, 
		_viewportX0,_viewportY0,_viewportWidth,_viewportHeight,
		frameSize.xMin(),frameSize.xMax(),frameSize.yMin(),frameSize.yMax());
	
	_movie->display();

	Render::endDisplay();
}

float Stage::getBackgroundAlpha() const
{
	return _backgroundColor.getA() / 255.0f;
}

void Stage::setBackgroundColor(const RGBA& color)
{
	_backgroundColor = color;
}

void Stage::setBackgroundAlpha(float alpha)
{
	_backgroundColor.setA(Base::iclamp(Base::frnd(alpha * 255.0f), 0, 255));
}

void Stage::setDisplayViewport(int x0, int y0, int w, int h)
{
	_viewportX0 = x0;
	_viewportY0 = y0;
	_viewportWidth = w;
	_viewportHeight = h;

	// Recompute pixel scale.
	RECT frameSize = _movieDef->getFrameSize();
	float	scale_x = _viewportWidth / TWIPS_TO_PIXELS(frameSize.width());
	float	scale_y = _viewportHeight / TWIPS_TO_PIXELS(frameSize.height());
	_pixelScale = fmax(scale_x, scale_y);
}