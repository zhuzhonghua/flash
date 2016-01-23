#ifndef Z_MOVIE_DEFINITION_
#define Z_MOVIE_DEFINITION_

#include "movie_definition.h"
#include "type/rect.h"

#include <vector>
#include <map>

class ZFile;
class Stream;
class ExecuteTag;
class Context;

class MovieDefinitionImpl : public MovieDefinition
{
public:
	MovieDefinitionImpl(Context* ctx);
	~MovieDefinitionImpl();
public:
	bool	read(ZFile* file);
	void	readTags();

	virtual void	addExecuteTag(ExecuteTag* c);
	virtual void	addInitAction(ExecuteTag* c);
	virtual Stage*	createInstance();
			Stage*	createStage();

	virtual int		getVersion() const;

	virtual const std::vector<ExecuteTag*>*		getPlaylist(int frameNumber);
	virtual const std::vector<ExecuteTag*>*		getInitActions(int frameNumber);

	RECT			getFrameSize() const { return _frameSize; }

	void			addCharacter(int id, CharacterDefinition* ch);
private:
	int			_version;
	int			_fileLen;
	int			_fileEndPos;

	int			_loadedLength;

	Stream*		_str;
	ZFile*		_zlibFile;

	RECT		_frameSize;
	float		_frameRate;	

	std::vector<std::vector<ExecuteTag*> >		_playList;
	std::vector<std::vector<ExecuteTag*> >		_initActionList;

	std::map<int, CharacterDefinition*>			_characters;
};

#endif