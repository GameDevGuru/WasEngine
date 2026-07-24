//**************************************************************************
//	Daniel Waseem, Copyright 2020
//
//************************************************************************
#include "stdafx.h"
#include "Renderer.h"

#include <gl/GL.h>
#include <cmath>
#include <cstdio>

#pragma comment(lib, "opengl32.lib")

//---------------------------------------------------------------------------
// Minimal hand-rolled GL 3.3 core / WGL extension surface. gl/GL.h only
// exposes OpenGL 1.1 (no VAOs/VBOs/shaders), so the function pointers and
// constants used by this renderer are declared here and resolved at
// runtime via wglGetProcAddress once a context is current.
//---------------------------------------------------------------------------
#pragma region GL Constants
#define GL_ARRAY_BUFFER            0x8892
#define GL_STATIC_DRAW             0x88E4
#define GL_FRAGMENT_SHADER         0x8B30
#define GL_VERTEX_SHADER           0x8B31
#define GL_COMPILE_STATUS          0x8B81
#define GL_LINK_STATUS             0x8B82
#define GL_DEPTH_BUFFER_BIT_       0x00000100

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#pragma endregion

#pragma region GL Function Pointer Typedefs
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int* attribList);

typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, ptrdiff_t size, const void* data, GLenum usage);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef GLuint(APIENTRY* PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const char** string, const GLint* length);
typedef void (APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRY* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef GLuint(APIENTRY* PFNGLCREATEPROGRAMPROC)();
typedef void (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (APIENTRY* PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint(APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const char* name);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
#pragma endregion

namespace
{
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

	PFNGLGENVERTEXARRAYSPROC        glGenVertexArrays = nullptr;
	PFNGLBINDVERTEXARRAYPROC        glBindVertexArray = nullptr;
	PFNGLDELETEVERTEXARRAYSPROC     glDeleteVertexArrays = nullptr;
	PFNGLGENBUFFERSPROC             glGenBuffers = nullptr;
	PFNGLBINDBUFFERPROC             glBindBuffer = nullptr;
	PFNGLBUFFERDATAPROC             glBufferData = nullptr;
	PFNGLDELETEBUFFERSPROC          glDeleteBuffers = nullptr;
	PFNGLVERTEXATTRIBPOINTERPROC    glVertexAttribPointer = nullptr;
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
	PFNGLCREATESHADERPROC           glCreateShader = nullptr;
	PFNGLSHADERSOURCEPROC           glShaderSource = nullptr;
	PFNGLCOMPILESHADERPROC          glCompileShader = nullptr;
	PFNGLGETSHADERIVPROC            glGetShaderiv = nullptr;
	PFNGLGETSHADERINFOLOGPROC       glGetShaderInfoLog = nullptr;
	PFNGLCREATEPROGRAMPROC          glCreateProgram = nullptr;
	PFNGLATTACHSHADERPROC           glAttachShader = nullptr;
	PFNGLLINKPROGRAMPROC            glLinkProgram = nullptr;
	PFNGLGETPROGRAMIVPROC           glGetProgramiv = nullptr;
	PFNGLGETPROGRAMINFOLOGPROC      glGetProgramInfoLog = nullptr;
	PFNGLDELETESHADERPROC           glDeleteShader = nullptr;
	PFNGLDELETEPROGRAMPROC          glDeleteProgram = nullptr;
	PFNGLUSEPROGRAMPROC             glUseProgram = nullptr;
	PFNGLGETUNIFORMLOCATIONPROC     glGetUniformLocation = nullptr;
	PFNGLUNIFORMMATRIX4FVPROC       glUniformMatrix4fv = nullptr;

	template <typename T>
	bool LoadProc(T& outProc, const char* name)
	{
		outProc = reinterpret_cast<T>(wglGetProcAddress(name));
		return outProc != nullptr;
	}

	//-----------------------------------------------------------------------
	// Minimal column-major 4x4 float matrix helpers (OpenGL convention).
	//-----------------------------------------------------------------------
	void Mat4Identity(float* m)
	{
		for (int i = 0; i < 16; ++i) m[i] = 0.0f;
		m[0] = m[5] = m[10] = m[15] = 1.0f;
	}

	void Mat4Multiply(const float* a, const float* b, float* out)
	{
		float result[16];
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				float sum = 0.0f;
				for (int k = 0; k < 4; ++k)
				{
					sum += a[k * 4 + row] * b[col * 4 + k];
				}
				result[col * 4 + row] = sum;
			}
		}
		for (int i = 0; i < 16; ++i) out[i] = result[i];
	}

	void Mat4Perspective(float fovYRadians, float aspect, float nearZ, float farZ, float* out)
	{
		Mat4Identity(out);
		float f = 1.0f / tanf(fovYRadians / 2.0f);
		out[0] = f / aspect;
		out[5] = f;
		out[10] = (farZ + nearZ) / (nearZ - farZ);
		out[11] = -1.0f;
		out[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
		out[15] = 0.0f;
	}

	void Mat4Translate(float x, float y, float z, float* out)
	{
		Mat4Identity(out);
		out[12] = x;
		out[13] = y;
		out[14] = z;
	}

	void Mat4RotateY(float angleRadians, float* out)
	{
		Mat4Identity(out);
		float c = cosf(angleRadians);
		float s = sinf(angleRadians);
		out[0] = c;
		out[2] = -s;
		out[8] = s;
		out[10] = c;
	}

	const char* VertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aColor;\n"
		"uniform mat4 uMVP;\n"
		"out vec3 vColor;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = uMVP * vec4(aPos, 1.0);\n"
		"    vColor = aColor;\n"
		"}\n";

	const char* FragmentShaderSource =
		"#version 330 core\n"
		"in vec3 vColor;\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"    FragColor = vec4(vColor, 1.0);\n"
		"}\n";
}

Renderer::Renderer()
	: m_hwnd(nullptr)
	, m_hdc(nullptr)
	, m_hglrc(nullptr)
	, m_shaderProgram(0)
	, m_vao(0)
	, m_vbo(0)
	, m_vertexCount(0)
	, m_mvpUniformLocation(-1)
	, m_viewportWidth(800)
	, m_viewportHeight(600)
	, m_rotationAngle(0.0f)
	, m_offsetX(0.0f)
	, m_offsetY(0.0f)
	, m_moveLeft(false)
	, m_moveRight(false)
	, m_moveUp(false)
	, m_moveDown(false)
{
}

Renderer::~Renderer()
{
	Shutdown();
}

bool Renderer::Init(HWND hwnd)
{
	m_hwnd = hwnd;

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	m_viewportWidth = clientRect.right - clientRect.left;
	m_viewportHeight = clientRect.bottom - clientRect.top;

	if (!InitPixelFormatAndContext(hwnd))
	{
		return false;
	}

	if (!LoadGLExtensions())
	{
		return false;
	}

	if (!CompileShaders())
	{
		return false;
	}

	SetupTriangle();

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_viewportWidth, m_viewportHeight);

	return true;
}

bool Renderer::InitPixelFormatAndContext(HWND hwnd)
{
	m_hdc = GetDC(hwnd);
	if (!m_hdc)
	{
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
	if (pixelFormat == 0 || !SetPixelFormat(m_hdc, pixelFormat, &pfd))
	{
		return false;
	}

	// Create a temporary legacy context so we can look up wglCreateContextAttribsARB.
	HGLRC dummyContext = wglCreateContext(m_hdc);
	if (!dummyContext || !wglMakeCurrent(m_hdc, dummyContext))
	{
		return false;
	}

	LoadProc(wglCreateContextAttribsARB, "wglCreateContextAttribsARB");

	HGLRC realContext = nullptr;
	if (wglCreateContextAttribsARB)
	{
		int attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
		realContext = wglCreateContextAttribsARB(m_hdc, nullptr, attribs);
	}

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(dummyContext);

	if (!realContext)
	{
		// Fall back to the legacy context if the ARB path isn't available.
		realContext = wglCreateContext(m_hdc);
	}

	if (!realContext || !wglMakeCurrent(m_hdc, realContext))
	{
		return false;
	}

	m_hglrc = realContext;
	return true;
}

bool Renderer::LoadGLExtensions()
{
	bool ok = true;
	ok &= LoadProc(glGenVertexArrays, "glGenVertexArrays");
	ok &= LoadProc(glBindVertexArray, "glBindVertexArray");
	ok &= LoadProc(glDeleteVertexArrays, "glDeleteVertexArrays");
	ok &= LoadProc(glGenBuffers, "glGenBuffers");
	ok &= LoadProc(glBindBuffer, "glBindBuffer");
	ok &= LoadProc(glBufferData, "glBufferData");
	ok &= LoadProc(glDeleteBuffers, "glDeleteBuffers");
	ok &= LoadProc(glVertexAttribPointer, "glVertexAttribPointer");
	ok &= LoadProc(glEnableVertexAttribArray, "glEnableVertexAttribArray");
	ok &= LoadProc(glCreateShader, "glCreateShader");
	ok &= LoadProc(glShaderSource, "glShaderSource");
	ok &= LoadProc(glCompileShader, "glCompileShader");
	ok &= LoadProc(glGetShaderiv, "glGetShaderiv");
	ok &= LoadProc(glGetShaderInfoLog, "glGetShaderInfoLog");
	ok &= LoadProc(glCreateProgram, "glCreateProgram");
	ok &= LoadProc(glAttachShader, "glAttachShader");
	ok &= LoadProc(glLinkProgram, "glLinkProgram");
	ok &= LoadProc(glGetProgramiv, "glGetProgramiv");
	ok &= LoadProc(glGetProgramInfoLog, "glGetProgramInfoLog");
	ok &= LoadProc(glDeleteShader, "glDeleteShader");
	ok &= LoadProc(glDeleteProgram, "glDeleteProgram");
	ok &= LoadProc(glUseProgram, "glUseProgram");
	ok &= LoadProc(glGetUniformLocation, "glGetUniformLocation");
	ok &= LoadProc(glUniformMatrix4fv, "glUniformMatrix4fv");
	return ok;
}

bool Renderer::CompileShaders()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &VertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	GLint success = 0;
	char infoLog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
		OutputDebugStringA("Vertex shader compile error: ");
		OutputDebugStringA(infoLog);
		return false;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &FragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
		OutputDebugStringA("Fragment shader compile error: ");
		OutputDebugStringA(infoLog);
		return false;
	}

	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, vertexShader);
	glAttachShader(m_shaderProgram, fragmentShader);
	glLinkProgram(m_shaderProgram);

	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_shaderProgram, sizeof(infoLog), nullptr, infoLog);
		OutputDebugStringA("Shader program link error: ");
		OutputDebugStringA(infoLog);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_mvpUniformLocation = glGetUniformLocation(m_shaderProgram, "uMVP");

	return true;
}

void Renderer::SetupTriangle()
{
	// Square-based pyramid: apex + 4 base corners, built as 6 independent
	// triangles (4 side faces + 2 base triangles) so no index buffer is
	// needed. Interleaved position (x, y, z) + color (r, g, b) per vertex.
	const float apex[3] = { 0.0f,  0.6f,  0.0f };
	const float baseFL[3] = { -0.5f, -0.4f, -0.5f };
	const float baseFR[3] = { 0.5f, -0.4f, -0.5f };
	const float baseBR[3] = { 0.5f, -0.4f,  0.5f };
	const float baseBL[3] = { -0.5f, -0.4f,  0.5f };

	const float frontColor[3] = { 1.0f, 0.2f, 0.2f };
	const float rightColor[3] = { 0.2f, 1.0f, 0.2f };
	const float backColor[3] = { 0.2f, 0.4f, 1.0f };
	const float leftColor[3] = { 1.0f, 0.9f, 0.2f };
	const float baseColor[3] = { 0.6f, 0.2f, 0.8f };

#define PYR_VERT(p, c) p[0], p[1], p[2], c[0], c[1], c[2]

	float vertices[] =
	{
		// Front face (apex, front-left, front-right)
		PYR_VERT(apex, frontColor),
		PYR_VERT(baseFL, frontColor),
		PYR_VERT(baseFR, frontColor),

		// Right face (apex, front-right, back-right)
		PYR_VERT(apex, rightColor),
		PYR_VERT(baseFR, rightColor),
		PYR_VERT(baseBR, rightColor),

		// Back face (apex, back-right, back-left)
		PYR_VERT(apex, backColor),
		PYR_VERT(baseBR, backColor),
		PYR_VERT(baseBL, backColor),

		// Left face (apex, back-left, front-left)
		PYR_VERT(apex, leftColor),
		PYR_VERT(baseBL, leftColor),
		PYR_VERT(baseFL, leftColor),

		// Base, triangle 1 (front-left, back-right, front-right)
		PYR_VERT(baseFL, baseColor),
		PYR_VERT(baseBR, baseColor),
		PYR_VERT(baseFR, baseColor),

		// Base, triangle 2 (front-left, back-left, back-right)
		PYR_VERT(baseFL, baseColor),
		PYR_VERT(baseBL, baseColor),
		PYR_VERT(baseBR, baseColor),
	};

#undef PYR_VERT

	m_vertexCount = static_cast<int>(sizeof(vertices) / (6 * sizeof(float)));

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Renderer::Shutdown()
{
	if (m_hglrc)
	{
		wglMakeCurrent(m_hdc, m_hglrc);

		if (m_vbo && glDeleteBuffers) glDeleteBuffers(1, &m_vbo);
		if (m_vao && glDeleteVertexArrays) glDeleteVertexArrays(1, &m_vao);
		if (m_shaderProgram && glDeleteProgram) glDeleteProgram(m_shaderProgram);

		m_vbo = 0;
		m_vao = 0;
		m_shaderProgram = 0;

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(m_hglrc);
		m_hglrc = nullptr;
	}

	if (m_hdc && m_hwnd)
	{
		ReleaseDC(m_hwnd, m_hdc);
		m_hdc = nullptr;
	}
}

void Renderer::Resize(int width, int height)
{
	m_viewportWidth = width > 0 ? width : 1;
	m_viewportHeight = height > 0 ? height : 1;

	if (m_hglrc)
	{
		wglMakeCurrent(m_hdc, m_hglrc);
		glViewport(0, 0, m_viewportWidth, m_viewportHeight);
	}
}

void Renderer::Update(float deltaTime)
{
	const float rotationSpeed = 1.5f; // radians per second
	const float moveSpeed = 1.0f;     // units per second

	m_rotationAngle += deltaTime * rotationSpeed;
	if (m_rotationAngle > 6.2831853f)
	{
		m_rotationAngle -= 6.2831853f;
	}

	if (m_moveLeft)  m_offsetX -= moveSpeed * deltaTime;
	if (m_moveRight) m_offsetX += moveSpeed * deltaTime;
	if (m_moveUp)    m_offsetY += moveSpeed * deltaTime;
	if (m_moveDown)  m_offsetY -= moveSpeed * deltaTime;

	// Keep the triangle roughly within the visible frame.
	if (m_offsetX < -1.5f) m_offsetX = -1.5f;
	if (m_offsetX > 1.5f)  m_offsetX = 1.5f;
	if (m_offsetY < -1.5f) m_offsetY = -1.5f;
	if (m_offsetY > 1.5f)  m_offsetY = 1.5f;
}

void Renderer::SetMoveInput(bool left, bool right, bool up, bool down)
{
	m_moveLeft = left;
	m_moveRight = right;
	m_moveUp = up;
	m_moveDown = down;
}

void Renderer::Render()
{
	if (!m_hglrc)
	{
		return;
	}

	wglMakeCurrent(m_hdc, m_hglrc);

	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shaderProgram);

	float model[16], rotation[16], translation[16];
	Mat4RotateY(m_rotationAngle, rotation);
	Mat4Translate(m_offsetX, m_offsetY, 0.0f, translation);
	Mat4Multiply(translation, rotation, model);

	float view[16];
	Mat4Translate(0.0f, 0.0f, -3.0f, view);

	float aspect = (m_viewportHeight != 0) ? static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight) : 1.0f;
	float projection[16];
	Mat4Perspective(0.785398f /* 45 degrees */, aspect, 0.1f, 100.0f, projection);

	float viewModel[16];
	Mat4Multiply(view, model, viewModel);

	float mvp[16];
	Mat4Multiply(projection, viewModel, mvp);

	glUniformMatrix4fv(m_mvpUniformLocation, 1, GL_FALSE, mvp);

	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
	glBindVertexArray(0);

	SwapBuffers(m_hdc);
}
