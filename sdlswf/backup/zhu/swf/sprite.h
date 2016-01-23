#ifndef Z_SPRITE_
#define Z_SPRITE_

#include "character.h"

class Context;
class MovieDefinition;
class Stage;

class Sprite : public Character
{
public:
	// Unique id of a gameswf resource
	enum { _class_id = AS_SPRITE };
	virtual bool is(int class_id) const
	{
		if (_class_id == class_id) return true;
		else return Character::is(class_id);
	}

	Sprite(Context* context, MovieDefinition* def, Stage* r, Character* parent, int id);
	virtual ~Sprite();

	void					executeFrameTags(int frame, bool stateOnly = false);

	virtual void			advance(double dt);
	virtual void			display();

	virtual float			getBackgroundAlpha() const;
	virtual void			setBackgroundAlpha(float alpha);
	virtual void			setBackgroundColor(const RGBA& color);
private:
	MovieDefinition*		_def;
	Stage*					_stage;

	std::vector<bool>		_initActionsExecuted;
};
#endif