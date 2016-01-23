/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, standard IO, and, strings
#include <SDL.h>
#include "SDL_gpu.h"
#include <stdio.h>
#include <string>

#include "sdl2_file.h"
#include "sdl2_render_handler.h"
#include "sdl2_sound_handler.h"

#include "gameswf/gameswf.h"
#include "gameswf/gameswf_player.h"
#include "gameswf/gameswf_root.h"

//Screen dimension constants
const int SCREEN_WIDTH = 550;
const int SCREEN_HEIGHT = 400;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Input handler
void handleKeys( unsigned char key, int x, int y );

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
//SDL_Window* gWindow = NULL;
//SDL_Renderer* renderer = NULL;

GPU_Target* screen;
bool init()
{
	//Initialization flag
	bool success = true;

	////Initialize SDL
	//if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	//{
	//	printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
	//	success = false;
	//}
	//else
	//{
	//	//Create window
	//	gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	//	if( gWindow == NULL )
	//	{
	//		printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
	//		success = false;
	//	}
	//	else
	//	{
	//		//Create context
	//		renderer = SDL_CreateRenderer(gWindow, -1, 0);
	//		if (renderer == NULL)
	//		{
	//			printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
	//			success = false;
	//		}
	//	}
	//}
	GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
	screen = GPU_Init(SCREEN_WIDTH, SCREEN_HEIGHT, GPU_DEFAULT_INIT_FLAGS);
	return success;
}

void handleKeys( unsigned char key, int x, int y )
{
}

void update()
{
	//No per frame update needed
}

void doRender()
{
	//SDL_RenderPresent(renderer);
	GPU_Flip(screen);
}

void close()
{
	//Destroy window	
	//SDL_DestroyWindow( gWindow );
	//gWindow = NULL;

	//Quit SDL subsystems
	//SDL_Quit();
	GPU_Quit();
}

tu_file* SDL2FileOpener(const char* path)
{
	return SDL2File::createSDL2File(path);
}

gameswf::render_handler* CreateSDL2RenderHandler()
// Factory.
{
	return new SDL2RenderHandler(screen);
}

int main( int argc, char** args )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		gameswf::register_file_opener_callback(SDL2FileOpener);
		
		gameswf::render_handler*	render = new SDL2RenderHandler(screen);
		gameswf::set_render_handler(render);
		render->open();
		
		gameswf::sound_handler*	sound = new SDL2SoundHandler();
		gameswf::set_sound_handler(sound);

		gameswf::gc_ptr<gameswf::player> player = new gameswf::player();
		gameswf::gc_ptr<gameswf::root>	m = player->load_file(args[1]);

		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;
		
		//Enable text input
		SDL_StartTextInput();

		double lastTime = SDL_GetTicks();
		//While application is running
		while( !quit )
		{
			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//User requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
				}
				//Handle keypress with current mouse position
				else if( e.type == SDL_TEXTINPUT )
				{
					int x = 0, y = 0;
					SDL_GetMouseState( &x, &y );
					handleKeys( e.text.text[ 0 ], x, y );
				}
			}

			m = player->get_root();
			double time = SDL_GetTicks();

			m->advance(time - lastTime);
			m->display();
			lastTime = time;

			//Render quad
			//doRender();

			SDL_Delay(10);
		}
		
		//Disable text input
		SDL_StopTextInput();

		gameswf::set_render_handler(NULL);
		delete render;

		gameswf::set_sound_handler(NULL);
		delete sound;
	}	

	//Free resources and close SDL
	close();

	return 0;
}
