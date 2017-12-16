//
//  main.cpp
//  Final
//
//  Created by Janina Soriano on 11/30/17.
//  Copyright (c) 2017 Janina Soriano. All rights reserved.
//


#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "Matrix.h"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cassert>
#include <vector>
#include <SDL_mixer.h>
#include <stdio.h>
#include <Entity.h>
#include <Sheetsprite/Sheetsprite.h>
#include <Game.cpp>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>


#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;


int main(int argc, char *argv[])
{
    ShaderProgram program = Setup();
    
    bool done = false;
    while (!done)
    {
        
        //Time
        ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        ProcessInput(done);
        Update(elapsed);
        Render(program);
        
        SDL_GL_SwapWindow(displayWindow);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    SDL_Quit();
    return 0;
}