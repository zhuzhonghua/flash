#include "sprite.h"

#include "movie_definition.h"
#include "execute_tag.h"
#include "stage.h"

Sprite::Sprite(Context* context, MovieDefinition* def, Stage* r, Character* parent, int id)
:Character(context, parent, id)
{
	_def = def;
	_stage = r;

	_initActionsExecuted.resize(def->getLoadingFrame());
}

Sprite::~Sprite()
{

}

void Sprite::executeFrameTags(int frame, bool stateOnly)
{
	if (_def->getVersion() < 9)
	{
		if (_initActionsExecuted[frame] == false)
		{
			_initActionsExecuted[frame] = true;
			const std::vector<ExecuteTag*>* initActions = _def->getInitActions(frame);
			for (std::vector<ExecuteTag*>::const_iterator iter = initActions->begin();
				iter != initActions->end(); ++iter)
			{
				(*iter)->execute(this);
			}
		}
	}	

	const std::vector<ExecuteTag*>*	playList = _def->getPlaylist(frame);
	for (std::vector<ExecuteTag*>::const_iterator iter = playList->begin();
		iter != playList->end(); ++iter)
	{
		if (stateOnly)
		{
			(*iter)->executeState(this);
		}
		else
		{
			(*iter)->execute(this);
		}		
	}
}

void Sprite::advance(double dt)
{

}

void Sprite::display()
{

}

float Sprite::getBackgroundAlpha() const
{
	return _stage->getBackgroundAlpha();
}

void Sprite::setBackgroundAlpha(float alpha)
{
	_stage->setBackgroundAlpha(alpha);
}

void Sprite::setBackgroundColor(const RGBA& color)
{
	_stage->setBackgroundColor(color);
}