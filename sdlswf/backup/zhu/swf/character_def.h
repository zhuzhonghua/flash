#ifndef ZCHARACTER_DEF_H
#define ZCHARACTER_DEF_H

#include "as_object_interface.h"

class Context;

class CharacterDefinition : public ASObjectInterface
{
public:
	enum { _class_id = AS_CHARACTER_DEF };
	virtual bool is(int class_id) const
	{
		return _class_id == class_id;
	}

	CharacterDefinition(Context* ctx);
	Context*			getContext() { return _context; }
private:
	int			_id;
	Context*	_context;
};
#endif