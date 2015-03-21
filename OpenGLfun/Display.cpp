#include "Display.h"
#include <iostream>
#include <GL/glew.h>


Display::Display(int width, int height, const std::string& title)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);


    m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_RENDERER_PRESENTVSYNC);
    m_glContext = SDL_GL_CreateContext(m_window);


    GLenum status = glewInit();
    if(status != GLEW_OK)
    {
        std::cout << glewGetErrorString(status) << std::endl;
    }
    m_isOpen = true;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}


Display::~Display(void)
{
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Display::IsOpen()
{
    return m_isOpen;
}

void Display::Update()
{
    SDL_GL_SwapWindow(m_window);
    SDL_Event e;

    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
            m_isOpen = false;
    }
}

void Display::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
}

int Display::GetHeight()
{
	int w;
    int h;
	int* p_w = &w;
	int* p_h = &h;
    SDL_GetWindowSize(m_window, p_w, p_h);

	return h;
}

int Display::GetWidth()
{
	int w;
	int h;
	int* p_w = &w;
	int* p_h = &h;
    SDL_GetWindowSize(m_window, p_w, p_h);

	return w;
}
