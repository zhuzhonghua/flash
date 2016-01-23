#ifndef Z_TAG_LOADER_
#define Z_TAG_LOADER_

#include <boost/noncopyable.hpp>
#include <map>

#include "stream.h"

class MovieDefinition;
typedef void(*LoaderFunction)(Stream* input, const TagInfo& info, MovieDefinition* m);

class TagLoader : boost::noncopyable
{
public:
	static TagLoader*	get();	
	LoaderFunction		getLoader(int tag);
protected:
	void addLoader(int tag_type, LoaderFunction lf);
	TagLoader();
private:
	std::map<int, LoaderFunction>	_loaders;
	static TagLoader* _s_inst;
};

void endLoader(Stream* in, const TagInfo& info, MovieDefinition* m);
void setBackgroundColorLoader(Stream* in, const TagInfo& info, MovieDefinition* m);
void defineShapeLoader(Stream* in, const TagInfo& info, MovieDefinition* m);

#endif