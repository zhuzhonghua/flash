#ifndef ZSTAGE_H
#define ZSTAGE_H
#include "type/rgba.h"

class Character;
class Context;
class MovieDefinitionImpl;

class Stage
{
public:
	Stage(Context* ctx, MovieDefinitionImpl* def);
	virtual ~Stage();

	void			setStageMovie(Character* movie);

	Character*		getStageMovie() { return _movie; }

	void			advance(double dt);
	void			display();
	float			getBackgroundAlpha() const;
	void			setBackgroundColor(const RGBA& color);
	void			setBackgroundAlpha(float alpha);

	void			setDisplayViewport(int x0, int y0, int w, int h);
private:
	Character*		_movie;
	Context*		_ctx;
	MovieDefinitionImpl*	_movieDef;

	RGBA			_backgroundColor;

	int				_viewportX0, _viewportY0, _viewportWidth, _viewportHeight;
	float			_pixelScale;
};
#endif