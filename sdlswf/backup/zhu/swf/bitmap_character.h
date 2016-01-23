#ifndef _BITMAP_CHARACTER_H
#define _BITMAP_CHARACTER_H

#include "character_def.h"
#include "base/memory_state.h"

class BitmapInfo;
class MovieDefinition;
class RECT;
class Character;

class BitmapCharacterDef : public CharacterDefinition, public MemoryState<BitmapCharacterDef>
{
public:
	BitmapCharacterDef(Context* ctx) :
	CharacterDefinition(ctx)
	{
	}

	virtual BitmapInfo*	getBitmapInfo() = 0;
};

// Bitmap character
class BitmapCharacter : public BitmapCharacterDef
{
public:
	BitmapCharacter(MovieDefinition* rdef, BitmapInfo* bi);

	// Return true if the specified point is on the interior of our shape.
	// Incoming coords are local coords.
			bool	pointTestLocal(float x, float y);

	virtual void	getBound(RECT* bound);
	virtual void	display(Character* ch);
	BitmapInfo*		getBitmapInfo();

private:
	BitmapInfo*		_bitmapInfo;
};

#endif