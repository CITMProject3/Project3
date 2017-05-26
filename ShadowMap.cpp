#include "ShadowMap.h"

#include "Glew\include\glew.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ComponentCamera.h"
#include "MasterRender.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "DebugDraw.h"

using namespace math;

ShadowMap::ShadowMap()
{
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
	frustum.SetOrthographic(5, 7);

	frustum.SetViewPlaneDistances(0.3, 500);
	frustum.SetPos(float3(0, 0, 0));
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::Init(int width, int height)
{
	this->width = width;
	this->height = height;

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenTextures(1, &shadow_map_id);
	glBindTexture(GL_TEXTURE_2D, shadow_map_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map_id, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::CleanUp()
{
	glDeleteFramebuffers(1, &fbo_id);
	glDeleteTextures(1, &shadow_map_id);
}

void ShadowMap::Render(const float3& light_dir, const std::vector<GameObject*>& entities)
{
	//Update cam matrix
	UpdateShadowBox(light_dir);

	GLint old_viewport[4];
	glGetIntegerv(GL_VIEWPORT, old_viewport);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	ShadowShader shadow_shader = App->renderer3D->ms_render->shadow_shader;

	glUseProgram(shadow_shader.id);

	float4x4 vpMatrix = frustum.ProjectionMatrix().Transposed() * float4x4(frustum.ViewMatrix()).Transposed();
	glUniformMatrix4fv(shadow_shader.mv_matrix, 1, GL_FALSE, *(vpMatrix).v);

	for (vector<GameObject*>::const_iterator entity = entities.begin(); entity != entities.end(); ++entity)
	{
		glUniformMatrix4fv(shadow_shader.model, 1, GL_FALSE, *((*entity)->GetGlobalMatrix().Transposed()).v);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, (*entity)->mesh_to_draw->id_vertices);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*entity)->mesh_to_draw->id_indices);
		glDrawElements(GL_TRIANGLES, (*entity)->mesh_to_draw->num_indices, GL_UNSIGNED_INT, (void*)0);
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
}

void ShadowMap::UpdateShadowBox(const float3& light_dir)
{
	
	vec up = Quat::RotateFromTo(vec(0, 0, 1), light_dir) * vec(0, 1, 0);
	
	frustum.SetFront(light_dir.Normalized());
	frustum.SetUp(up.Normalized());

	//Testing
	vec cam_pos = frustum.Pos();
	g_Debug->AddLine(cam_pos, cam_pos + frustum.Up() * 2, g_Debug->red, 2.0f);
	g_Debug->AddLine(cam_pos, cam_pos + frustum.Front() * 2, g_Debug->green, 2.0f);
}
