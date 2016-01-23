#include "character.h"

Character::Character(Context* ctx, Character* parent, int id)
:ASObject(ctx)
{
	_parent = parent;
	_id = id;
}

Character::~Character()
{

}