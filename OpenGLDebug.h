#ifndef __OPENGLDEBUG_H__
#define __OPENGLDEBUG_H__


#include "Glew\include\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "glut\glut.h"
#include <string>

class OpenGLDebug
{
public:
	OpenGLDebug();
	~OpenGLDebug();

	static void CALLBACK OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data);

private:
	static std::string GetStringForType(GLenum type);
	static std::string GetStringForSource(GLenum source);
	static std::string GetStringForSeverity(GLenum severity);

};
#endif // !__OPENGLDEBUG_H__
