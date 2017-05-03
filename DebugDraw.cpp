#include "Application.h"
#include "DebugDraw.h"

#include "Glew\include\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Time.h"

//#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
//#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
//#pragma comment (lib, "Glew/libx86/glew32.lib") 

DebugDraw::DebugDraw(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

DebugDraw::~DebugDraw()
{
	
}

bool DebugDraw::Start()
{
	//Create the base primitives
	CreateBaseLine();
	CreateBaseCube();
	CreateBaseCross();
	CreateBaseRect();
	CreateBaseArrow();

	return true;
}

update_status DebugDraw::PreUpdate()
{
	//Update timers
	std::list<DebugPrimitive*>::iterator d_primitive = draw_list.begin();

	while (d_primitive != draw_list.end())
	{
		//One frame duration
		if ((*d_primitive)->life == 0)
		{
			d_primitive = RemovePrimitive(d_primitive);
			continue;
		}

		(*d_primitive)->life -= time->RealDeltaTime();

		if ((*d_primitive)->life <= 0.0f)
		{
			d_primitive = RemovePrimitive(d_primitive);
			continue;
		}
		d_primitive++;
	}

	return UPDATE_CONTINUE;
}

update_status DebugDraw::PostUpdate()
{
	Draw();
	return UPDATE_CONTINUE;
}

bool DebugDraw::CleanUp()
{
	std::list<DebugPrimitive*>::iterator d_primitive = draw_list.begin();

	while (d_primitive != draw_list.end())
	{
		delete (*d_primitive);
		d_primitive++;
	}

	draw_list.clear();

	return true;
}

void DebugDraw::CreateBaseLine()
{
	// 1
	// | /
	// 0 ---

	GLfloat vertices[]
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint indices[]
	{
		0, 1
	};

	num_indices_line = 2;

	glGenBuffers(1, (GLuint*)&id_vertices_line);
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 2, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*)&id_indices_line);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices_line);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 2, indices, GL_STATIC_DRAW);
}

void DebugDraw::CreateBaseCube()
{
	//  6------- 5
	// /       / |
	// 2------1  4
	// |      | /
	// 3----- 0

	GLfloat vertices[] =
	{
		0.5f, -0.5f,  0.5f, //0
		0.5f,  0.5f,  0.5f, //1
		-0.5f,  0.5f,  0.5f, //2
		-0.5f, -0.5f,  0.5f, //3
		0.5f, -0.5f, -0.5f, //4
		0.5f,  0.5f, -0.5f, //5
		-0.5f,  0.5f, -0.5f, //6
		-0.5f, -0.5f, -0.5f, //7
	};

	GLuint indices[] =
	{
		3, 0,
		0, 1,
		1, 2,
		2, 3,
		7, 4,
		4, 5,
		5, 6,
		6, 7,
		0, 4,
		1, 5,
		3, 7,
		2, 6
	};

	num_indices_cube = 24;

	glGenBuffers(1, (GLuint*)&id_vertices_cube);
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*)&id_indices_cube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices_cube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_cube, indices, GL_STATIC_DRAW);
}

void DebugDraw::CreateBaseCross()
{
	// 7  1  5		13    11
	//  \ | /		 \ | /
	// 2--8-9--3  	-----
	//  / | \		 / | \
	// 4  0  6		10    12

	GLfloat vertices[] =
	{
		 0.0f, -0.5f,  0.0f, //0
		 0.0f,  0.5f,  0.0f, //1
		-0.5f,  0.0f,  0.0f, //2
		 0.5f,  0.0f,  0.0f, //3
		-0.4f, -0.4f,  0.4f, //4
		 0.4f,  0.4f, -0.4f, //5
		 0.4f, -0.4f,  0.4f, //6
		-0.4f,  0.4f, -0.4f, //7
		 0.0f,  0.0f,  0.5f, //8
		 0.0f,  0.0f, -0.5f, //9
		-0.4f, -0.4f, -0.4f, //10
		 0.4f,  0.4f,  0.4f, //11
		 0.4f, -0.4f, -0.4f, //12
		-0.4f,  0.4f,  0.4f //13
	};

	GLuint indices[] = 
	{
		0,1,2,3,4,5,6,7,8,9,10,11,12,13
	};

	num_indices_cross = 14;

	glGenBuffers(1, (GLuint*)&id_vertices_cross);
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices_cross);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 14, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*)&id_indices_cross);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices_cross);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_cross, indices, GL_STATIC_DRAW);

}

void DebugDraw::CreateBaseRect()
{
	//                           Y
	// 3 -- 2                    |
	// |    |  Y is the normal    -->X
	// 0----1                   /
	//                          Z

	GLfloat vertices[] =
	{
		-0.5f, 0.0f,  0.5f,
		 0.5f, 0.0f,  0.5f,
		 0.5f, 0.0f, -0.5f,
		-0.5f, 0.0f, -0.5f
	};

	GLuint indices[] =
	{
		0, 1, 1, 2, 2, 3, 3, 0
	};

	num_indices_rect = 8;

	glGenBuffers(1, (GLuint*)&id_vertices_rect);
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices_rect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*)&id_indices_rect);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices_rect);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_rect, indices, GL_STATIC_DRAW);
}

void DebugDraw::CreateBaseArrow()
{
	//  1
	//2 | 3
	//  |
	//  0 

	GLfloat vertices[]
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		-0.2f, 0.8f, 0.0f,
		0.2f, 0.8f, 0.0f

	};

	GLuint indices[]
	{
		0, 1,
		1, 2,
		1, 3,
	};

	num_indices_arrow = 6;

	glGenBuffers(1, (GLuint*)&id_vertices_arrow);
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices_arrow);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*)&id_indices_arrow);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices_arrow);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_arrow, indices, GL_STATIC_DRAW);
}

void DebugDraw::AddCross(const float3 & point, math::float3 color, float size, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_cross, id_indices_cross, num_indices_cross);

	float scale = (size <= 0) ? 1.0f : size;

	d_prim->global_matrix = d_prim->global_matrix.FromTRS(point, math::Quat::identity, scale * math::vec::one);

	draw_list.push_back(d_prim);
}

void DebugDraw::AddLine(const float3 & from_position, const float3 & to_position, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_line, id_indices_line, num_indices_line);

	math::vec vec1 = math::float3(0, 1, 0);
	math::vec vec2 = to_position - from_position;

	float length = vec2.Length();

	math::Quat rot = rot.RotateFromTo(vec1.Normalized(), vec2.Normalized());

	if (rot.Equals(math::Quat(1, 0, 0, 0))) //Same direction
		if (vec2.y > 0)
			rot = math::Quat::identity; //Facing Up
		else
			rot = math::Quat::RotateX(math::pi); //Facing Down

	d_prim->global_matrix = d_prim->global_matrix.FromTRS(from_position, rot, math::float3(1, length, 1));

	draw_list.push_back(d_prim);

}

void DebugDraw::AddAABB(const math::AABB& aabb, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_cube, id_indices_cube, num_indices_cube);

	d_prim->global_matrix = d_prim->global_matrix.FromTRS(aabb.CenterPoint(), math::Quat::identity, aabb.Size());

	draw_list.push_back(d_prim);
}

void DebugDraw::AddAABB(const math::float3& min_point,const math::float3& max_point, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_cube, id_indices_cube, num_indices_cube);

	math::AABB aabb;
	aabb.minPoint = min_point;
	aabb.maxPoint = max_point;
	
	d_prim->global_matrix = d_prim->global_matrix.FromTRS(aabb.CenterPoint(), math::Quat::identity, aabb.Size());

	draw_list.push_back(d_prim);
}

//WRONG!!! Needs X rotation too
void DebugDraw::AddRect(const math::float3 & center_point, const math::float3& normal, const math::float2 size, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_rect, id_indices_rect, num_indices_rect);

	math::float3 origin = math::float3(0, 1, 0);
	origin.Normalize();
	math::float3 normal_vec = normal.Normalized();

	math::Quat rotation;
	rotation = rotation.RotateFromTo(origin, normal_vec);

	d_prim->global_matrix = d_prim->global_matrix.FromTRS(center_point, rotation, math::float3(size.x, 1, size.y));

	draw_list.push_back(d_prim);
}

void DebugDraw::AddFrustum(const math::Frustum & frustum, float fake_far_dst, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	/*          6------7
		2----3  |      |
		|    |  |      |
		0----1  4------5

		near       far
	*/

	math::vec corners[8];
	frustum.GetCornerPoints(corners);

	//Near face
	AddLine(corners[0], corners[1], color, line_width, duration, depth_enabled);
	AddLine(corners[1], corners[3], color, line_width, duration, depth_enabled);
	AddLine(corners[3], corners[2], color, line_width, duration, depth_enabled);
	AddLine(corners[2], corners[0], color, line_width, duration, depth_enabled);

	//Calculate fake far corners
	math::vec far_vec4, far_vec5, far_vec6, far_vec7;

	far_vec4 = (corners[4] - corners[0]).Normalized() * (fake_far_dst) + frustum.Pos();
	far_vec5 = (corners[5] - corners[1]).Normalized() * (fake_far_dst) + frustum.Pos();
	far_vec6 = (corners[6] - corners[2]).Normalized() * (fake_far_dst) + frustum.Pos();
	far_vec7 = (corners[7] - corners[3]).Normalized() * (fake_far_dst) + frustum.Pos();

	//Far face
	AddLine(far_vec4, far_vec5, color, line_width, duration, depth_enabled);
	AddLine(far_vec5, far_vec7, color, line_width, duration, depth_enabled);
	AddLine(far_vec7, far_vec6, color, line_width, duration, depth_enabled);
	AddLine(far_vec6, far_vec4, color, line_width, duration, depth_enabled);

	//Planes connections
	AddLine(corners[0], far_vec4, color, line_width, duration, depth_enabled);
	AddLine(corners[1], far_vec5, color, line_width, duration, depth_enabled);
	AddLine(corners[3], far_vec7, color, line_width, duration, depth_enabled);
	AddLine(corners[2], far_vec6, color, line_width, duration, depth_enabled);
	
}

void DebugDraw::AddArrow2(const math::float3 & from_position, const math::float3& to_position, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_arrow, id_indices_arrow, num_indices_arrow);

	math::vec vec1 = math::float3(0, 1, 0);
	math::vec vec2 = to_position - from_position;

	float length = vec2.Length();

	math::Quat rot = rot.RotateFromTo(vec1.Normalized(), vec2.Normalized());

	if (rot.Equals(math::Quat(1, 0, 0, 0))) //Same direction
		if (vec2.y > 0)
			rot = math::Quat::identity; //Facing Up
		else
			rot = math::Quat::RotateX(math::pi); //Facing Down

	d_prim->global_matrix = d_prim->global_matrix.FromTRS(from_position, rot, math::float3(1, length, 1));

	draw_list.push_back(d_prim);
}

void DebugDraw::AddArrow(const math::float3 & origin, const math::float3& direction, math::float3 color, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_arrow, id_indices_arrow, num_indices_arrow);

	math::vec vec1 = math::float3(0, 1, 0);
	math::vec vec2 = direction.Normalized();

	math::Quat rot = rot.RotateFromTo(vec1.Normalized(), vec2.Normalized());

	if (rot.Equals(math::Quat(1, 0, 0, 0))) //Same direction
		if (vec2.y > 0)
			rot = math::Quat::identity; //Facing Up
		else
			rot = math::Quat::RotateX(math::pi); //Facing Down

	d_prim->global_matrix = d_prim->global_matrix.FromTRS(origin, rot, math::float3(1, 1, 1));

	draw_list.push_back(d_prim);
}

void DebugDraw::AddOBB(const math::OBB & obb, math::float3 color, float size, float line_width, float duration, bool depth_enabled)
{
	DebugPrimitive* d_prim = new DebugPrimitive();

	FillCommonPrimitiveValues(d_prim, color, line_width, duration, depth_enabled, id_vertices_cube, id_indices_cube, num_indices_cube);

	d_prim->global_matrix =  obb.LocalToWorld();

	draw_list.push_back(d_prim);

}

void DebugDraw::Draw()
{
	
	std::list<DebugPrimitive*>::iterator item = draw_list.begin();
	glDisable(GL_LIGHTING);
	for (item; item != draw_list.end(); item++)
	{
		glColor3f((*item)->color.x, (*item)->color.y, (*item)->color.z);
		glLineWidth((*item)->line_width);

		glPushMatrix();
		glMultMatrixf(*(*item)->global_matrix.Transposed().v);

		glEnableClientState(GL_VERTEX_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, (*item)->vertices_id);
		glVertexPointer(3, GL_FLOAT, 0, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*item)->indices_id);
		glDrawElements(GL_LINES, (*item)->num_indices, GL_UNSIGNED_INT, NULL);


		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
	}
	//glColor3f(1, 1, 1); //Reset color. 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

std::list<DebugPrimitive*>::iterator DebugDraw::RemovePrimitive(std::list<DebugPrimitive*>::iterator & it)
{
	std::list<DebugPrimitive*>::iterator next = std::next(it);

	delete *it;
	draw_list.erase(it);

	return next;
}

void DebugDraw::FillCommonPrimitiveValues(DebugPrimitive* primitive, float3 color, float line_width, float duration, bool depth_enabled, unsigned int vertices_id, unsigned int indices_id, unsigned int num_indices)
{
	primitive->color = color;
	primitive->line_width = line_width;
	primitive->life = duration;
	primitive->depth_enabled = depth_enabled;
	primitive->vertices_id = vertices_id;
	primitive->indices_id = indices_id;
	primitive->num_indices = num_indices;
}