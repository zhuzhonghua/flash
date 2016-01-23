#include "sdl2_file.h"


static int sdl_read_func(void* dst, int bytes, void* appdata)
// Return the number of bytes actually read.  EOF or an error would
// cause that to not be equal to "bytes".
{
	assert(appdata);
	assert(dst);
	return (int)SDL_RWread((SDL_RWops*)appdata, dst, 1, bytes);
}

static int sdl_write_func(const void* src, int bytes, void* appdata)
// Return the number of bytes actually written.
{
	assert(appdata);
	assert(src);
	return (int)SDL_RWwrite((SDL_RWops*)appdata, src, 1, bytes);
}

static int sdl_seek_func(int pos, void *appdata)
// Return 0 on success, or TU_FILE_SEEK_ERROR on failure.
{
	assert(appdata);
	clearerr((FILE*)appdata);	// make sure EOF flag is cleared.
	int	result = (int)SDL_RWseek((SDL_RWops*)appdata, pos, RW_SEEK_SET);
	if (result == EOF)
	{
		// @@ TODO should set m_error to something relevant based on errno.
		return TU_FILE_SEEK_ERROR;
	}
	return 0;
}

static int sdl_seek_to_end_func(void *appdata)
// Return 0 on success, TU_FILE_SEEK_ERROR on failure.
{
	assert(appdata);
	int	result = (int)SDL_RWseek((SDL_RWops*)appdata, 0, RW_SEEK_END);
	if (result == EOF)
	{
		// @@ TODO should set m_error to something relevant based on errno.
		return TU_FILE_SEEK_ERROR;
	}
	return 0;
}

static int sdl_tell_func(const void *appdata)
// Return the file position, or -1 on failure.
{
	assert(appdata);
	return SDL_RWtell((SDL_RWops*)appdata);
}

static bool sdl_get_eof_func(void *appdata)
// Return true if we're at EOF.
{
	assert(false);
	return false;
}

static int sdl_close_func(void *appdata)
// Return 0 on success, or TU_FILE_CLOSE_ERROR on failure.
{
	assert(appdata);	
	int	result = SDL_RWclose((SDL_RWops*)appdata); 
	if (result == EOF)
	{
		// @@ TODO should set m_error to something relevant based on errno.
		return TU_FILE_CLOSE_ERROR;
	}
	return 0;
}

SDL2File* SDL2File::createSDL2File(const char* path)
{
	SDL_RWops* rwops = SDL_RWFromFile(path, "rb");
	return new SDL2File(rwops);
}

SDL2File::SDL2File(SDL_RWops*	rwops) :
tu_file(rwops,
	sdl_read_func,
	sdl_write_func,
	sdl_seek_func,
	sdl_seek_to_end_func,
	sdl_tell_func,
	sdl_get_eof_func,
	sdl_close_func)
{
	_rwops = rwops;

	m_data = (void *)_rwops;
}

SDL2File::~SDL2File()
{

}