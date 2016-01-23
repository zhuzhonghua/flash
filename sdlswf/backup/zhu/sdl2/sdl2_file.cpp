#include "global.h"

#include "sdl2_file.h"

SDL2File::SDL2File(const char* path)
{
	_rwops = SDL_RWFromFile(path, "rb");
}

SDL2File::~SDL2File()
{

}

int	SDL2File::getPosition() const
{
	return SDL_RWtell(_rwops);
}

void SDL2File::setPosition(int pos)
{
	SDL_RWseek(_rwops, pos, RW_SEEK_SET);
}

int SDL2File::readBytes(void* bytes, int numBytes)
{
	return SDL_RWread(_rwops, bytes, 1, numBytes);
}