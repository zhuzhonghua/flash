#ifndef Z_AS_OBJECT_
#define Z_AS_OBJECT_

#include "as_object_interface.h"

class Context;

class ASObject : public ASObjectInterface
{
public:
	enum	{ _class_id = AS_OBJECT };
	virtual bool is(int class_id) const
	{
		return _class_id == class_id;
	}

	ASObject(Context* ctx);
	virtual ~ASObject();

	Context*		getContext() { return _ctx; }
private:
	Context*		_ctx;
};
#endif