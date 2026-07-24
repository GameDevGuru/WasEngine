//**************************************************************************
//	Daniel Waseem, Copyright 2020
//
//************************************************************************
#pragma once

#include <Windows.h>

// Minimal OpenGL 3.3 core-profile renderer that draws a single rotating,
// arrow-key-movable triangle through a vertex + fragment shader pair.
// No external GL loader / math library is used; the small set of GL 3.3
// entry points and matrix helpers needed are implemented in Renderer.cpp.
class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Init(HWND hwnd);
	void Shutdown();
	void Resize(int width, int height);
	void Update(float deltaTime);
	void Render();
	void SetMoveInput(bool left, bool right, bool up, bool down);

private:
	bool InitPixelFormatAndContext(HWND hwnd);
	bool LoadGLExtensions();
	bool CompileShaders();
	void SetupTriangle();

private:
	HWND  m_hwnd;
	HDC   m_hdc;
	HGLRC m_hglrc;

	unsigned int m_shaderProgram;
	unsigned int m_vao;
	unsigned int m_vbo;
	int          m_vertexCount;
	int          m_mvpUniformLocation;

	int m_viewportWidth;
	int m_viewportHeight;

	float m_rotationAngle;
	float m_offsetX;
	float m_offsetY;

	bool m_moveLeft;
	bool m_moveRight;
	bool m_moveUp;
	bool m_moveDown;
};
