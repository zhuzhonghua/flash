#include "movie_definition.h"

MovieDefinition::MovieDefinition(Context* ctx) : CharacterDefinition(ctx)
{
	_loadingFrame = 0;
}

void MovieDefinition::setFrameCount(int c)
{
	_frameCount = c;
	if (_frameCount <= 0)
	{
		_frameCount = 1;
	}
}

int MovieDefinition::getFrameCount()
{
	return _frameCount;
}

void MovieDefinition::incLoadingFrame()
{
	_loadingFrame++;
}

int MovieDefinition::getLoadingFrame()
{
	return _loadingFrame;
}