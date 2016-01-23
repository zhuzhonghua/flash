/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, standard IO, and, strings
#include <SDL.h>
#include <stdio.h>
#include <string>

#include "global.h"
#include "swf/zplayer.h"
#include "system.h"

#include "sdl2_render_handler.h"
#include "sdl2_file.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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
SDL_Window* gWindow = NULL;
SDL_Renderer* renderer = NULL;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create context
			renderer = SDL_CreateRenderer(gWindow, -1, 0);
			if (renderer == NULL)
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
		}
	}

	return success;
}

void handleKeys( unsigned char key, int x, int y )
{
}

void update()
{
	//No per frame update needed
}

void render()
{
	SDL_RenderPresent(renderer);
}

void close()
{
	//Destroy window	
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

ZFile* SDL2FileOpener(const char* path)
{
	return new SDL2File(path);
}

RenderHandler* getSDL2RenderHandler(SDL_Renderer* r)
{
	return new SDL2RenderHandler(r);
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
		System::Config* config = System::Config::get();
		config->setFileOpener(SDL2FileOpener);
		config->setRenderHandler(getSDL2RenderHandler(renderer));

		ZPlayer z;
		z.loadFile(args[1]);
		z.setDisplayViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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

			double time = SDL_GetTicks();
			z.tick(time-lastTime);
			lastTime = time;

			//Render quad
			render();			
		}
		
		//Disable text input
		SDL_StopTextInput();
	}

	//Free resources and close SDL
	close();

	return 0;
}
