#ifndef Z_SDL2_FILE_
#define Z_SDL2_FILE_

#include "base/tu_file.h"
#include <SDL.h>

class SDL2File : public tu_file
{
public:
	static SDL2File* createSDL2File(const char* name);
	SDL2File(SDL_RWops*	rwops);
	~SDL2File();
private:
	SDL_RWops*	_rwops;
};

#endif