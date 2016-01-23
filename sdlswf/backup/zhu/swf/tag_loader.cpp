#include "tag_loader.h"
#include "global.h"
#include "set_background_color.h"
#include "movie_definition.h"
#include "shape_character_def.h"

TagLoader* TagLoader::_s_inst;

TagLoader* TagLoader::get()
{
	if (!_s_inst)
	{
		_s_inst = new TagLoader();
	}
	return _s_inst;
}

LoaderFunction TagLoader::getLoader(int tag)
{
	std::map<int, LoaderFunction>::iterator iter=_loaders.find(tag);
	if (iter != _loaders.end())
	{
		return iter->second;
	}
	else
	{
		return NULL;
	}
}

void TagLoader::addLoader(int tag, LoaderFunction lf)
{
	_loaders[tag] = lf;
}

TagLoader::TagLoader()
{
	addLoader(SWF::END,					endLoader);
	addLoader(SWF::SETBACKGROUNDCOLOR,	setBackgroundColorLoader);
}

void endLoader(Stream* in, const TagInfo& info, MovieDefinition* m)
{
	
}

void setBackgroundColorLoader(Stream* in, const TagInfo& info, MovieDefinition* m)
{
	SetBackgroundColor* t = new SetBackgroundColor();
	t->read(in);
	m->addExecuteTag(t);
}

void defineShapeLoader(Stream* in, const TagInfo& info, MovieDefinition* m)
{
	Assert(info.tagType == SWF::DEFINESHAPE || info.tagType == SWF::DEFINESHAPE2 || 
		info.tagType == SWF::DEFINESHAPE3 || info.tagType == SWF::DEFINESHAPE4);

	UInt16 chId = in->readUI16();
	
	ShapeCharacterDefinition* ch = ZNEW ShapeCharacterDefinition(m->getContext());
	ch->read(in, info.tagType, true, m);

	m->addCharacter(chId, ch);
}