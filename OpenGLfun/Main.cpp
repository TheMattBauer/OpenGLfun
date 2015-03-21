#include <iostream>
#include <fstream>
#include <GL\glew.h>
#include "Display.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Transform.h"
#include "Camera.h"
#include <list>
#include <windows.h>
#include <SDL2\SDL.h>

#define WIDTH 800
#define HEIGHT 600

static inline std::vector<std::string> SplitString(const std::string &s, char delim)
{
    std::vector<std::string> elems;
        
    const char* cstr = s.c_str();
    unsigned int strLength = s.length();
    unsigned int start = 0;
    unsigned int end = 0;
        
    while(end <= strLength)
    {
        while(end <= strLength)
        {
            if(cstr[end] == delim)
                break;
            end++;
        }
            
        elems.push_back(s.substr(start, end - start));
        start = end + 1;
        end = start;
    }
        
    return elems;
}


INT OpenFileDialog(PCHAR FileName)
{ 
	FileName[0] = 0;
	OPENFILENAME  ofn;        
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = NULL;
	ofn.hInstance       = NULL;
	ofn.lpstrFilter     = "All files (*.*)\0*.*\0";   
	ofn.lpstrFile       = FileName;
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrTitle      = "Please Select A File To Open";
	ofn.Flags           = OFN_NONETWORKBUTTON |
							OFN_FILEMUSTEXIST |
							OFN_HIDEREADONLY  |
							OFN_NOCHANGEDIR ;
	if (!GetOpenFileName(&ofn))
		return 0;
	return 1; 
}

std::list<Camera> LoadCameras()
{
	std::list<Camera> cameraList;
	Camera currentCamera;
    std::ifstream file;
    file.open("./res/cameras_05.txt");

    std::string line;
    if(file.is_open())
    {
        while(file.good())
        {
            getline(file, line);
        
            unsigned int lineLength = line.length();
            
            const char* lineCStr = line.c_str();

			if (lineCStr[0] == 'c')
			{
				cameraList.push_back(currentCamera);
				currentCamera = Camera();
			}
			else if(lineCStr[0] == 'i')
			{
				currentCamera.SetName(line.substr(2, lineLength-1));
			}
			else if(lineCStr[0] == 't')
			{
				currentCamera.SetIsParallel (line.substr(2, lineLength-1) == "parallel");
			}
			else if(lineCStr[0] == 'e')
			{
				std::vector<std::string> tokens = SplitString(line.substr(2, lineLength-1), ' ');
				glm::vec3 vector(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]));
				currentCamera.SetEye(vector);
			}
			else if(lineCStr[0] == 'l')
			{
				std::vector<std::string> tokens = SplitString(line.substr(2, lineLength-1), ' ');
				glm::vec3 vector(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]));
				currentCamera.SetLookAt(vector);
			}
			else if(lineCStr[0] == 'u')
			{
				std::vector<std::string> tokens = SplitString(line.substr(2, lineLength-1), ' ');
				glm::vec3 vector(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]));
				currentCamera.SetVup(vector);
			}
			else if(lineCStr[0] == 'w')
			{
				std::vector<std::string> tokens = SplitString(line.substr(2, lineLength-1), ' ');
				currentCamera.SetPerspective(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]), stof(tokens[3]), stof(tokens[4]), stof(tokens[5]));
			}
			else if(lineCStr[0] == 's')
			{
				std::vector<std::string> tokens = SplitString(line.substr(2, lineLength-1), ' ');
				glm::vec4 vector(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
				currentCamera.SetViewport(vector);
			}
        }
    }
    else
    {
        std::cerr << "Unable to load cameras: " << std::endl;
    }

	cameraList.push_back(currentCamera);
	cameraList.pop_front();
	return cameraList;
}

int main (int argc, char** argv)
{
	SDL_Event event;
	bool quit = false;

	Display display (WIDTH,HEIGHT, "yo");

	std::list<Camera> cameraList;
	cameraList = LoadCameras();

	char fileName[MAX_PATH];
	OpenFileDialog(fileName);
	std::cout << "FileName: "<< fileName << std::endl;
	Mesh& mesh = Mesh(fileName);
	Mesh* mesh_p = &mesh;

	Shader shader("./res/basicShader");
	Texture texture("./res/bricks.jpg");
	Transform transform;

	long last = 0;
	float deltaTime = 0.0;

    while (display.IsOpen() && !quit)
    {

		long now = SDL_GetTicks();
		deltaTime = ((float)(now - last)) / 1000;
		last = now;
		//std::cout << deltaTime << std::endl;

        display.Clear(0.09f,0.05f,0.05f,0);

		SDL_WaitEvent(&event);
		switch(event.type)
		{
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_x)
			{
				if(event.key.keysym.mod & KMOD_SHIFT)
				{
					transform.GetRot().x += .0872664626f;
				}
				else
				{
					transform.GetRot().x += -.0872664626f;
				}
			}
			else if(event.key.keysym.sym == SDLK_y)
			{
				if(event.key.keysym.mod & KMOD_SHIFT)
				{
					transform.GetRot().y += .0872664626f;
				}
				else
				{
					transform.GetRot().y += -.0872664626f;
				}
			}
			else if(event.key.keysym.sym == SDLK_z)
			{
				if(event.key.keysym.mod & KMOD_SHIFT)
				{
					transform.GetRot().z += .0872664626f;
				}
				else
				{
					transform.GetRot().z += -.0872664626f;
				}
			}
			else if(event.key.keysym.sym == SDLK_s)
			{
				if(event.key.keysym.mod & KMOD_SHIFT)
				{
					transform.GetScale().x -= transform.GetScale().x*.05f;
					transform.GetScale().y -= transform.GetScale().y*.05f;
					transform.GetScale().z -= transform.GetScale().z*.05f;
				}
				else
				{
					transform.GetScale().x += transform.GetScale().x*.05f;
					transform.GetScale().y += transform.GetScale().y*.05f;
					transform.GetScale().z += transform.GetScale().z*.05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_f)
			{
				for (Camera &cam : cameraList)
				{
					glm::vec3 movementVector = glm::normalize(cam.GetEye() - cam.GetLookAt());
					float distance = glm::distance(cam.GetEye(), cam.GetLookAt());
					cam.GetEye() -= movementVector * distance * .05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_b)
			{
				for (Camera &cam : cameraList)
				{
					glm::vec3 movementVector = glm::normalize(cam.GetEye() - cam.GetLookAt());
					float distance = glm::distance(cam.GetEye(), cam.GetLookAt());
					cam.GetEye() += movementVector * distance * .05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_LEFT)
			{
				for (Camera &cam : cameraList)
				{
					glm::vec3 forwardVector = glm::normalize(cam.GetEye() - cam.GetLookAt());
					glm::vec3 movementVector = glm::normalize(glm::cross(forwardVector, cam.GetVup()));

					float distance = glm::distance(cam.GetEye(), cam.GetLookAt());
					cam.GetEye() += movementVector * distance * .05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_RIGHT)
			{
				for (Camera &cam : cameraList)
				{
					glm::vec3 forwardVector = glm::normalize(cam.GetEye() - cam.GetLookAt());
					glm::vec3 movementVector = glm::normalize(glm::cross(forwardVector, cam.GetVup()));

					float distance = glm::distance(cam.GetEye(), cam.GetLookAt());
					cam.GetEye() -= movementVector * distance * .05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_UP)
			{
				for (Camera &cam : cameraList)
				{
					glm::vec3 forwardVector = glm::normalize(cam.GetEye() - cam.GetLookAt());
					glm::vec3 sideVector = glm::normalize(glm::cross(forwardVector, cam.GetVup()));
					glm::vec3 movementVector = glm::normalize(glm::cross(sideVector, forwardVector));

					cam.SetVup(movementVector);
					float distance = glm::distance(cam.GetEye(), cam.GetLookAt());
					cam.GetEye() += movementVector * distance * .05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_DOWN)
			{
				for (Camera &cam : cameraList)
				{
					glm::vec3 forwardVector = glm::normalize(cam.GetEye() - cam.GetLookAt());
					glm::vec3 sideVector = glm::normalize(glm::cross(forwardVector, cam.GetVup()));
					glm::vec3 movementVector = glm::normalize(glm::cross(sideVector, forwardVector));

					cam.SetVup(movementVector);
					float distance = glm::distance(cam.GetEye(), cam.GetLookAt());
					cam.GetEye() -= movementVector * distance * .05f;
				}
			}
			else if(event.key.keysym.sym == SDLK_p)
			{
				for (Camera &cam : cameraList)
				{
					cam.GetIsParallel() = !cam.GetIsParallel();
					cam.ResetPerspective();
				}
			}
			else if(event.key.keysym.sym == SDLK_n)
			{
				OpenFileDialog(fileName);
				std::cout << "FileName: "<< fileName << std::endl;
				Mesh& meshb = Mesh(fileName);
				mesh_p = &meshb;
			}
			break;
		}

		for (Camera cam : cameraList)
		{
			cam.ResetPerspective();
			cam.UpdateViewport(display.GetWidth(), display.GetHeight());
			glScissor (cam.GetViewport()[0] * display.GetWidth(),
					  (1.0-cam.GetViewport()[3]) * display.GetHeight(),
					  (cam.GetViewport()[2]-cam.GetViewport()[0]) * display.GetWidth(),
					  (cam.GetViewport()[3]-cam.GetViewport()[1]) * display.GetHeight());

			glEnable(GL_SCISSOR_TEST);
			display.Clear(1,1,1,0);
			glDisable(GL_SCISSOR_TEST);

			shader.Bind();
			texture.Bind(0);
			shader.Update(transform, cam);
			mesh_p->Draw();
		}

        display.Update();
    }

    return 0;
}