#include "global.h"
#include "movie_definition_impl.h"
#include "context.h"
#include "stage.h"
#include "sprite.h"

#include "base/zlib_file.h"
#include "tag_loader.h"

MovieDefinitionImpl::MovieDefinitionImpl(Context* ctx): MovieDefinition(ctx)
{
	_zlibFile = NULL;
}

MovieDefinitionImpl::~MovieDefinitionImpl()
{
	if (_zlibFile)
	{
		delete _zlibFile;
		_zlibFile = NULL;
	}

	for (int i = 0; i < _playList.size(); ++i)
	{
		for (int j = 0; j < _playList[i].size(); ++j)
		{
			delete _playList[i][j];
		}
		_playList[i].clear();
	}
	_playList.clear();

	for (int i = 0; i < _initActionList.size(); ++i)
	{
		for (int j = 0; j < _initActionList[i].size(); ++j)
		{
			delete _initActionList[i][j];
		}
		_initActionList[i].clear();
	}
	_initActionList.clear();
}

bool MovieDefinitionImpl::read(ZFile* file)
{
	int	fileStartPos = file->getPosition();

	UInt8 header[4] = { 0 };
	file->readBytes(header, 4);
	Assert(header[0]=='C' || header[0]=='F');
	Assert(header[1] == 'W');
	Assert(header[2] == 'S');

	_version = header[3];
	_fileLen = file->readUI32();

	_fileEndPos = fileStartPos + _fileLen;

	bool	compressed = (header[0] == 'C');
	if (compressed)
	{
		_zlibFile = new ZLibFile(file);
		_str = new Stream(_zlibFile);

		_fileEndPos = _fileLen - 8;
	}
	else
	{
		_str = new Stream(file);
	}

	_frameSize.read(_str);
	_frameRate = _str->readUI16() / 256.0f;

	setFrameCount(_str->readUI16());

	_playList.resize(_frameCount);
	_initActionList.resize(_frameCount);

	readTags();

	return true;
}

void MovieDefinitionImpl::readTags()
{
	while (_str->getPosition() < _fileEndPos)
	{
		TagInfo info;
		_str->openTag(&info);

		if (info.tagType == SWF::SHOWFRAME)
		{
			incLoadingFrame();
		}
		else
		{
			LoaderFunction lf = TagLoader::get()->getLoader(info.tagType);
			if (lf)
			{
				(*lf)(_str, info, this);
			}
		}
		_str->closeTag();

		_loadedLength = _str->getPosition();
	}
}

int MovieDefinitionImpl::getVersion() const
{ 
	return _version; 
}

void MovieDefinitionImpl::addExecuteTag(ExecuteTag* c)
{
	_playList[getLoadingFrame()].push_back(c);
}

void MovieDefinitionImpl::addInitAction(ExecuteTag* c)
{
	_initActionList[getLoadingFrame()].push_back(c);
}

Stage* MovieDefinitionImpl::createInstance()
{
	Stage* stage = createStage();

	stage->getStageMovie()->executeFrameTags(0);

	return stage;
}

Stage* MovieDefinitionImpl::createStage()
{
	Stage* stage = new Stage(getContext(), this);
	Sprite* sprite = new Sprite(getContext(), this, stage, NULL, -1);

	stage->setStageMovie(sprite);

	return stage;
}

const std::vector<ExecuteTag*>* MovieDefinitionImpl::getPlaylist(int frameNumber)
{
	return &_playList[frameNumber];
}

const std::vector<ExecuteTag*>* MovieDefinitionImpl::getInitActions(int frameNumber)
{
	return &_initActionList[frameNumber];
}

void MovieDefinitionImpl::addCharacter(int id, CharacterDefinition* ch)
{
	bool flag = _characters.insert(std::make_pair(id, ch)).second;
	Assert(flag);
}