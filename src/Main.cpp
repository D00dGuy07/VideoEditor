#include <iostream>

#include "oglfv2/Util/GLFWLoad.h"
// Enable Nvidia high performace graphics
#include <windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#include "Application.h"

int oglTestMain(void)
{
	// Create the window
	GLFWwindow* window = LoadGLFW(720, 1080);

	if (window == NULL)
		return -1;

	// Run application
	Application* app = new Application();
	app->Run(window);

	// Clean stuff up
	delete app;
	glfwTerminate();

	return 0;
}

int main()
{
    oglTestMain();
}