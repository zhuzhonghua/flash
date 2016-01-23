#include "sdl2_sound_handler.h"

SDL2SoundHandler::SDL2SoundHandler()
{

}

SDL2SoundHandler::~SDL2SoundHandler()
{

}

// loads external sound file, only .WAV for now
int	SDL2SoundHandler::load_sound(const char* url)
{	
	return 0;
}

// Called to create a sample.
int	SDL2SoundHandler::create_sound(void* data, int data_bytes,
	int sample_count, format_type format,
	int sample_rate, bool stereo)
{
	return 0;
}

void SDL2SoundHandler::append_sound(int sound_handle, void* data, int data_bytes)
{

}

// Play the index'd sample.
void SDL2SoundHandler::play_sound(gameswf::as_object* listener_obj, int sound_handle, int loop_count)
{

}

void SDL2SoundHandler::set_max_volume(int vol)
{

}

void SDL2SoundHandler::stop_sound(int sound_handle)
{

}

// this gets called when it's done with a sample.
void SDL2SoundHandler::delete_sound(int sound_handle)
{

}

// this will stop all sounds playing.
void SDL2SoundHandler::stop_all_sounds()
{

}

// returns the sound volume level as an integer from 0 to 100.
int	SDL2SoundHandler::get_volume(int sound_handle)
{
	return 0;
}

void SDL2SoundHandler::set_volume(int sound_handle, int volume)
{

}

void SDL2SoundHandler::attach_aux_streamer(gameswf::sound_handler::aux_streamer_ptr ptr,
	gameswf::as_object* netstream)
{

}

void SDL2SoundHandler::detach_aux_streamer(gameswf::as_object* netstream)
{

}

// Converts input data to the SDL output format.
void SDL2SoundHandler::cvt(short int** adjusted_data, int* adjusted_size, unsigned char* data, int size,
	int channels, int freq)
{

}

void SDL2SoundHandler::pause(int sound_handle, bool paused)
{

}

int SDL2SoundHandler::get_position(int sound_handle)
{
	return 0;
}