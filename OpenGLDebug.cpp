#include "OpenGLDebug.h"
#include "Globals.h"

OpenGLDebug::OpenGLDebug()
{
}

OpenGLDebug::~OpenGLDebug()
{
}

void OpenGLDebug::OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * msg, const void * data)
{
	LOG("****Debug Output****\n Source: %s \n Type: %s \n Severity: %s \n Debug call: %s\n *************", GetStringForSource(source).c_str(), GetStringForType(type).c_str(), GetStringForSeverity(severity).c_str(), msg);
}

std::string OpenGLDebug::GetStringForType(GLenum type)
{
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		return"Error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "Deprecated behavior";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "Undefined behavior";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "Portability issue";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "Performance issue";
	case GL_DEBUG_TYPE_MARKER:
		return "Stream annotation";
	case GL_DEBUG_TYPE_OTHER_ARB:
		return "Other";
	default:
		//assert(false);
		return "";
	}
}

std::string OpenGLDebug::GetStringForSource(GLenum source)
{
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "Window system";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "Shader compiler";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "Third party";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "Application";
	case GL_DEBUG_SOURCE_OTHER:
		return "Other";
	default:
		//assert(false);
		return "";
	}
}

std::string OpenGLDebug::GetStringForSeverity(GLenum severity)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		return "High";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "Medium";
	case GL_DEBUG_SEVERITY_LOW:
		return "Low";
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "Notification";
	default:
		//assert(false);
		return("");
	}
}
