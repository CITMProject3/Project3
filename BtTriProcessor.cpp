#include "BtTriProcessor.h"

#include "ModuleRenderer3D.h"

#include <gl/GL.h>
#include <gl/GLU.h>

BtTriPRocessor::BtTriPRocessor(): color(0.0f, 0.6f, 0.6f)
{
}

void BtTriPRocessor::processTriangle(btVector3* triangle, int partId, int triangleIndex)
{
	Triangle tri;
	tri.a = vec(triangle[0].m_floats);
	tri.b = vec(triangle[1].m_floats);
	tri.c = vec(triangle[2].m_floats);

	if (useWire)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glBegin(GL_TRIANGLES);
	glColor3fv(color.ptr());
	glVertex3fv(tri.a.ptr());
	glVertex3fv(tri.b.ptr());
	glVertex3fv(tri.c.ptr());	
	glNormal3fv(tri.NormalCCW().ptr());
	glEnd();

	if (useWire)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}