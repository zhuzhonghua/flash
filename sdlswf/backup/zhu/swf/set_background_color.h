#ifndef Z_SET_BACKGROUND_COLOR_
#define Z_SET_BACKGROUND_COLOR_

#include "swf/type/rgba.h"
#include "swf/execute_tag.h"

class Stream;
class Character;

class SetBackgroundColor: public ExecuteTag
{
public:
	void		read(Stream* in);
	void		execute(Character* m);
	void		executeState(Character* m);

private:
	RGBA		_color;
};
#endif