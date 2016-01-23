#include "zplayer.h"
#include "system.h"
#include "movie_definition_impl.h"
#include "context.h"
#include "stage.h"

ZPlayer::ZPlayer()
{
	_file = NULL;
	_movie = NULL;

	_context = new Context();
}
ZPlayer::~ZPlayer()
{
	if (_file)
	{
		delete _file;
	}
	_file = NULL;

	if (_movie)
	{
		delete _movie;
	}
	_movie = NULL;

	delete _context;
}

void ZPlayer::loadFile(const char* fileName)
{
	_infile = fileName;

	_movie = this->createMovie(fileName);

	_stage = _movie->createInstance();
}

MovieDefinition* ZPlayer::createMovie(const char* filename)
{
	FileOpener lf = System::Config::get()->getFileOpener();

	_file = lf(filename);

	MovieDefinitionImpl* m = new MovieDefinitionImpl(this->getContext());
	m->read(_file);

	return m;
}

void ZPlayer::tick(double delta)
{
	_stage->advance(delta);
	_stage->display();
}

void ZPlayer::setDisplayViewport(int x0, int y0, int w, int h)
{
	_stage->setDisplayViewport(x0, y0, w, h);
}