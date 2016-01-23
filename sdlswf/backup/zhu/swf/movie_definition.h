#ifndef MOVIE_DEFINITION_
#define MOVIE_DEFINITION_

#include <vector>

#include "character_def.h"

class ExecuteTag;
class Stage;
class Context;
class BitmapCharacterDef;

class MovieDefinition: public CharacterDefinition
{
public:
	MovieDefinition(Context* ctx);

	void		setFrameCount(int c);
	int			getFrameCount();

	void		incLoadingFrame();
	int			getLoadingFrame();

	virtual void	addExecuteTag(ExecuteTag* c) = 0;
	virtual void	addInitAction(ExecuteTag* c) = 0;
	virtual Stage*	createInstance() = 0;
	virtual int		getVersion() const = 0;

	virtual const std::vector<ExecuteTag*>*		getPlaylist(int frameNumber) = 0;
	virtual const std::vector<ExecuteTag*>*		getInitActions(int frameNumber) = 0;

	virtual void					addCharacter(int id, CharacterDefinition* ch) = 0;
	virtual CharacterDefinition*	getCharacterDef(int id) = 0;

	virtual BitmapCharacterDef*		getBitmapCharacterDef(int character_id) = 0;
	virtual void					addBitmapCharacterDef(int character_id, BitmapCharacterDef* ch) = 0;
protected:
	int			_frameCount;
	int			_loadingFrame;
};
#endif