#ifndef ZPLAYER_H
#define ZPLAYER_H

#include <string>

class ZFile;
class MovieDefinition;
class Context;
class Stage;

class ZPlayer
{
public:
	ZPlayer();
	~ZPlayer();
	void				loadFile(const char* fileName);
	MovieDefinition*	createMovie(const char* filename);

	Context*			getContext() { return _context; }
	void				tick(double delta);
	void				setDisplayViewport(int x0, int y0, int w, int h);
private:
	std::string			_infile;
	ZFile*				_file;
	MovieDefinition*	_movie;
	Context*			_context;
	Stage*				_stage;
};

#endif