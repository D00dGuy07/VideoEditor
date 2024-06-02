#pragma once

#include "oglfv2/UI/Surface.h"

#include "oglfv2/Renderer/Mesh.h"
#include "oglfv2/Renderer/Shader.h"
#include "oglfv2/Renderer/Texture.h"
#include "oglfv2/Renderer/PixelBuffer.h"

struct GLFWwindow;

class Application
{
public:
	Application();

	void LoadResources();

	void Run(GLFWwindow* window);

private:
	GLFWwindow* m_Window;

	UI::Surface m_Surface;

	Mesh m_Mesh;
	Shader m_Shader;

	std::shared_ptr<Texture> m_Texture;
	PixelBuffer* m_PixelBuffer;

	void Draw();
};