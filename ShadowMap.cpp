#include "ShadowMap.h"

#include "Glew\include\glew.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ComponentCamera.h"
#include "MasterRender.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "DebugDraw.h"

//Testing
#include "ModuleWindow.h"

using namespace math;

ShadowMap::ShadowMap()
{
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
	frustum.SetOrthographic(800, 800);

	frustum.SetViewPlaneDistances(0.3, 200);
	frustum.SetPos(float3(0, 0, 0));

	frustum.SetFront(float3::unitZ);
	frustum.SetUp(float3::unitY);

	frustum.ComputeProjectionMatrix();
	frustum.ComputeViewMatrix();

	//Init frustrum
/*	float vertical_fov = DegToRad(60);
	float horizontal_fov = 2.0f*atanf(tanf(vertical_fov / 2.0f) * 1.0);

	frustum.SetPerspective(horizontal_fov, vertical_fov);
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);

	frustum.SetPos(float3(0, 0, 0));
	frustum.SetFront(float3::unitZ);
	frustum.SetUp(float3::unitY);
	

	frustum.SetViewPlaneDistances(0.3, 100);
	frustum.SetVerticalFovAndAspectRatio(DegToRad(60), 1.0f);*/
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map_id, 0);

	/*unsigned int texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::CleanUp()
{
	glDeleteFramebuffers(1, &fbo_id);
	glDeleteTextures(1, &shadow_map_id);
}

void ShadowMap::Render(const float4x4& light_matrix, const std::vector<GameObject*>& entities)
{
	//Update cam matrix
	UpdateShadowBox(light_matrix);

	GLint old_viewport[4];
	glGetIntegerv(GL_VIEWPORT, old_viewport);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	ShadowShader shadow_shader = App->renderer3D->ms_render->shadow_shader;

	glUseProgram(shadow_shader.id);

	glUniformMatrix4fv(shadow_shader.projection, 1, GL_FALSE, *frustum.ProjectionMatrix().Transposed().v);
	glUniformMatrix4fv(shadow_shader.view, 1, GL_FALSE, *(float4x4(frustum.ViewMatrix()).Transposed()).v);
	

	ComponentMesh* c_mesh = nullptr;

	for (vector<GameObject*>::const_iterator entity = entities.begin(); entity != entities.end(); ++entity)
	{
		glUniformMatrix4fv(shadow_shader.model, 1, GL_FALSE, *((*entity)->GetGlobalMatrix().Transposed()).v);

		c_mesh = (ComponentMesh*)(*entity)->GetComponent(C_MESH);
		if (c_mesh->HasBones())
		{
			glUniform1i(shadow_shader.has_anim, 1);
			glUniformMatrix4fv(shadow_shader.bones, c_mesh->bones_trans.size(), GL_FALSE, reinterpret_cast<GLfloat*>(c_mesh->bones_trans.data()));

			//Buffer bones id == 1
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, c_mesh->bone_id);
			glVertexAttribIPointer(1, 4, GL_INT, 0, (GLvoid*)0);

			//Buffer weights == 2
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, c_mesh->weight_id);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		}
		else
			glUniform1i(shadow_shader.has_anim, 0);

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

unsigned int ShadowMap::GetShadowMapId() const
{
	return shadow_map_id;
}

math::float4x4 ShadowMap::GetShadowView() const
{
	return float4x4(frustum.ViewMatrix()).Transposed();
}

math::float4x4 ShadowMap::GetShadowProjection() const
{
	return frustum.ProjectionMatrix().Transposed();
}

void ShadowMap::UpdateShadowBox(const float4x4& light_matrix)
{
	frustum.SetPos(light_matrix.TranslatePart());
	frustum.SetFront(light_matrix.WorldZ());
	frustum.SetUp(light_matrix.WorldY());
}
