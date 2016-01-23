#ifndef Z_SDL2_FILE_
#define Z_SDL2_FILE_

#include <base/file.h>

#include <SDL.h>

class SDL2File : public ZFile
{
public:
	SDL2File(const char* path);
	~SDL2File();
	virtual int	getPosition() const;
	virtual void setPosition(int pos);
	virtual int readBytes(void* bytes, int numBytes);
private:
	SDL_RWops*	_rwops;
};

#endif