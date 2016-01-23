// gameswf_sound_handler_sdl.h	-- Vitaly Alexeev <tishka92@yahoo.com> 2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef SDL2_SOUND_HANDLER_SDL_H
#define SDL2_SOUND_HANDLER_SDL_H

#include "gameswf/gameswf.h"

// Use SDL and ffmpeg to handle sounds.
struct SDL2SoundHandler : public gameswf::sound_handler
{
	SDL2SoundHandler();
	virtual ~SDL2SoundHandler();

	virtual bool is_open() { return false; };

	// loads external sound file, only .WAV for now
	virtual int	load_sound(const char* url);

	// Called to create a sample.
	virtual int	create_sound(void* data, int data_bytes,
		int sample_count, format_type format,
		int sample_rate, bool stereo);

	virtual void append_sound(int sound_handle, void* data, int data_bytes);

	// Play the index'd sample.
	virtual void	play_sound(gameswf::as_object* listener_obj, int sound_handle, int loop_count);

	virtual void	set_max_volume(int vol);

	virtual void	stop_sound(int sound_handle);

	// this gets called when it's done with a sample.
	virtual void	delete_sound(int sound_handle);

	// this will stop all sounds playing.
	virtual void	stop_all_sounds();

	// returns the sound volume level as an integer from 0 to 100.
	virtual int	get_volume(int sound_handle);

	virtual void	set_volume(int sound_handle, int volume);

	virtual void	attach_aux_streamer(gameswf::sound_handler::aux_streamer_ptr ptr,
		gameswf::as_object* netstream);
	virtual void	detach_aux_streamer(gameswf::as_object* netstream);

	// Converts input data to the SDL output format.
	virtual void cvt(short int** adjusted_data, int* adjusted_size, unsigned char* data, int size, 
		int channels, int freq);

	virtual void pause(int sound_handle, bool paused);
	virtual int get_position(int sound_handle);
};

#endif