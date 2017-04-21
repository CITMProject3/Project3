#ifndef __OPENGLFUNC_H__
#define __OPENGLFUNC_H__

namespace OpenGLFunc
{
	unsigned int CreateFBO(unsigned int width, unsigned int height, unsigned int& color_texture, unsigned int& depth_texture);
	unsigned int CreateColorTexture(unsigned int width, unsigned int height);
	unsigned int CreateDepthTexture(unsigned int width, unsigned int height);

	unsigned int CreateFBOColorOnly(unsigned int width, unsigned int height, unsigned int& color_texture);
	unsigned int CreateColorTextureSimple(unsigned int width, unsigned int height);

	void Bind(unsigned int fbo);
	void Unbind();

}

#endif // !__OPENGLFUNC_H__
