#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ComponentCamera.h"
#include "Glew\include\glew.h"
#include "SDL\include\SDL_opengl.h"
#include "GameObject.h"
#include "ModuleGOManager.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ResourceFileMaterial.h"
#include "ComponentLight.h"
#include "ResourceFileRenderTexture.h"
#include "Octree.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "Glew/libx86/glew32.lib") 

#include "Imgui\imgui.h"
#include "Imgui\imgui_impl_sdl_gl3.h"


ModuleRenderer3D::ModuleRenderer3D(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init(Data& config)
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->window);
	if(context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	GLenum gl_enum = glewInit();

	if (GLEW_OK != gl_enum)
	{
		LOG("Glew failed");
	}
	
	if(ret == true)
	{
		//Use Vsync
		if(VSYNC && SDL_GL_SetSwapInterval(1) < 0)
			LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.17f, 0.17f, 0.17f, 1.0f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		lights[0].Active(true);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
	}

	LOG("-------------Versions------------------");
	LOG("OpenGL Version: %s",glGetString(GL_VERSION));
	LOG("Glew Version: %s", glewGetString(GLEW_VERSION));

	// Projection matrix for
	OnResize(App->window->GetScreenWidth(), App->window->GetScreenHeight(), 60.0f);

	ImGui_ImplSdlGL3_Init(App->window->window);
	
	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(*App->camera->GetViewMatrix().v);

	// light 0 on cam pos
	lights[0].SetPos(App->camera->GetPosition().x, App->camera->GetPosition().y, App->camera->GetPosition().z);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	objects_to_draw.clear();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate()
{
	glEnable(GL_CLIP_DISTANCE0);
	//RenderTextures
	vector<ComponentCamera*> cameras;
	App->go_manager->GetAllCameras(cameras);
	for (int i = 0; i < cameras.size(); ++i)
	{
		if (cameras[i]->render_texture)
		{
			DrawScene(cameras[i], true);
		}
	}

	glDisable(GL_CLIP_DISTANCE0);
	//Current Camera
	ComponentCamera* current_cam = App->camera->GetCurrentCamera();
	DrawScene(current_cam);
	glUseProgram(0);

	ImGui::Render();
	SDL_GL_SwapWindow(App->window->window);
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");
	ImGui_ImplSdlGL3_Shutdown();
	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height, float fovy)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//Calculate perspective
	float4x4 perspective;
	float _near = 0.125f;
	float _far = 512.0f;

	perspective.SetIdentity();
	float tan_theta_over2 = tan(fovy * pi / 360.0f);

	perspective[0][0] = (1.0f / tan_theta_over2) / ((float) width / (float)height);
	perspective[1][1] = 1.0f / tan_theta_over2;
	perspective[2][2] = (_near + _far) / (_near - _far);
	perspective[3][2] = 2 * _near * _far / (_near - _far);
	perspective[2][3] = -1;
	perspective[3][3] = 0;

	ProjectionMatrix = perspective;
	glLoadMatrixf(*ProjectionMatrix.v);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	App->window->SetScreenSize(width, height);
	SendEvent(this, Event::WINDOW_RESIZE);
}

void ModuleRenderer3D::SetPerspective(const math::float4x4 & perspective)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ProjectionMatrix = perspective;
	glLoadMatrixf(*ProjectionMatrix.v);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ModuleRenderer3D::AddToDraw(GameObject* obj)
{
	if (obj)
	{
		if(obj->IsStatic() == false)
			objects_to_draw.push_back(obj);
	}
}

void ModuleRenderer3D::DrawScene(ComponentCamera* cam, bool has_render_tex) const
{
	if (has_render_tex)
	{
		cam->render_texture->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	int layer_mask = cam->GetLayerMask();
	//Draw Static GO
	vector<GameObject*> static_objects;
	App->go_manager->octree.Intersect(static_objects, *cam); //Culling for static objects

	for (vector<GameObject*>::iterator obj = static_objects.begin(); obj != static_objects.end(); ++obj)
	{
		if ((*obj)->IsActive()) //TODO: if component mesh is not active don't draw the object.
		{
			if (layer_mask == (layer_mask | (1 << (*obj)->layer)))
				Draw(*obj, App->lighting->GetLightInfo(), cam);
		}
	}

	//Draw dynamic GO
	for (vector<GameObject*>::const_iterator obj = objects_to_draw.begin(); obj != objects_to_draw.end(); ++obj)
	{
		if (cam->Intersects(*(*obj)->bounding_box))
		{
			if (layer_mask == (layer_mask | (1 << (*obj)->layer)))
				Draw((*obj), App->lighting->GetLightInfo(), cam);
		}
	}
	App->editor->skybox.Render(cam);

	if(has_render_tex)
		cam->render_texture->Unbind();
}

void ModuleRenderer3D::Draw(GameObject* obj, const LightInfo& light, ComponentCamera* cam) const
{
	ComponentMaterial* material = (ComponentMaterial*)obj->GetComponent(C_MATERIAL);

	if (material == nullptr)
		return;

	uint shader_id = 0;
	if (material->rc_material)
		shader_id = material->rc_material->GetShaderId();
	else
		shader_id = App->resource_manager->GetDefaultShaderId();

	//Use shader
	glUseProgram(shader_id);

	//Set uniforms

	//Matrices
	GLint model_location = glGetUniformLocation(shader_id, "model");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, *(obj->GetGlobalMatrix().Transposed()).v);
	GLint projection_location = glGetUniformLocation(shader_id, "projection");
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, *cam->GetProjectionMatrix().v);
	GLint view_location = glGetUniformLocation(shader_id, "view");
	glUniformMatrix4fv(view_location, 1, GL_FALSE, *cam->GetViewMatrix().v);	

	int count = 0;
	//Good code for textures. The code above must be removed.
	for (map<string, uint>::iterator tex = material->texture_ids.begin(); tex != material->texture_ids.end(); ++tex)
	{
		//Default first texture diffuse (if no specified)
		if ((*tex).first.size() == 0 && count == 0)
		{
			GLint has_tex_location = glGetUniformLocation(shader_id, "_HasTexture");
			glUniform1i(has_tex_location, 1);
			GLint texture_location = glGetUniformLocation(shader_id, "_Texture");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*tex).second);
			glUniform1i(texture_location, 0);
			count++;
			continue;
		}

		//Default second texture normal (if no specified)
		if ((*tex).first.size() == 0 && count == 1)
		{
			GLint has_normal_location = glGetUniformLocation(shader_id, "_HasNormalMap");
			glUniform1i(has_normal_location, 1);
			GLint texture_location = glGetUniformLocation(shader_id, "_HasNormalMap");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*tex).second);
			glUniform1i(texture_location, 1);
			count++;
			continue;
		}

		GLint tex_location = glGetUniformLocation(shader_id, (*tex).first.data());
		if (tex_location != -1)
		{
			glActiveTexture(GL_TEXTURE0 + count);
			glBindTexture(GL_TEXTURE_2D, (*tex).second);
			glUniform1i(tex_location, count);
			++count;
		}
	}

	

	//Textures
	/*if (material->GetDiffuseId() != 0)
	{
		GLint has_texture_location = glGetUniformLocation(shader_id, "_HasTexture");
		if (has_texture_location != -1)
		{
			glUniform1i(has_texture_location, 1);
		}
		GLint texture_location = glGetUniformLocation(shader_id, "_Texture");
		if (texture_location != -1)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material->GetDiffuseId());
			glUniform1i(texture_location, 0);
		}
	}
	else
	{
		GLint has_texture_location = glGetUniformLocation(shader_id, "_HasTexture");
		if (has_texture_location != -1)
		{
			glUniform1i(has_texture_location, 0);
		}
	}

	
	//Normal
	if (material->GetNormalId() != 0)
	{
		GLint has_normalmap_location = glGetUniformLocation(shader_id, "_HasNormalMap");
		if (has_normalmap_location != -1)
		{
			glUniform1i(has_normalmap_location, 1);
		}
		GLint normalmap_location = glGetUniformLocation(shader_id, "_NormalMap");
		if (normalmap_location != -1)
		{
			glUniform1i(normalmap_location, 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, material->GetNormalId());
		}
	}
	else
	{
		GLint has_normalmap_location = glGetUniformLocation(shader_id, "_HasNormalMap");
		if (has_normalmap_location != -1)
		{
			glUniform1i(has_normalmap_location, 0);
		}
	}*/

	//Lighting

	//Ambient
	GLint ambient_intensity_location = glGetUniformLocation(shader_id, "_AmbientIntensity");
	if (ambient_intensity_location != -1)
		glUniform1f(ambient_intensity_location, light.ambient_intensity);
	GLint ambient_color_location = glGetUniformLocation(shader_id, "_AmbientColor");
	if (ambient_color_location != -1)
		glUniform3f(ambient_color_location, light.ambient_color.x, light.ambient_color.y, light.ambient_color.z);

	//Directional
	if (light.has_directional)
	{
		GLint directional_intensity_location = glGetUniformLocation(shader_id, "_DirectionalIntensity");
		if (directional_intensity_location != -1)
			glUniform1f(directional_intensity_location, light.directional_intensity);
		GLint directional_color_location = glGetUniformLocation(shader_id, "_DirectionalColor");
		if (directional_color_location != -1)
			glUniform3f(directional_color_location, light.directional_color.x, light.directional_color.y, light.directional_color.z);
		GLint directional_direction_location = glGetUniformLocation(shader_id, "_DirectionalDirection");
		if (directional_direction_location != -1)
			glUniform3f(directional_direction_location, light.directional_direction.x, light.directional_direction.y, light.directional_direction.z);
	}
	
	
	//Other uniforms
	if (material->rc_material)
	{
		for (vector<Uniform*>::const_iterator uni = material->rc_material->material.uniforms.begin(); uni != material->rc_material->material.uniforms.end(); ++uni)
		{
			GLint uni_location = glGetUniformLocation(shader_id, (*uni)->name.data());
			
			if(uni_location != -1)
			switch ((*uni)->type)
			{
			case UniformType::U_BOOL:
			{
				glUniform1i(uni_location, *reinterpret_cast<bool*>((*uni)->value));
			}
				break;
			case U_INT:
			{
				glUniform1i(uni_location, *reinterpret_cast<int*>((*uni)->value));
			}
				break;
			case U_FLOAT:
			{
				glUniform1f(uni_location, *reinterpret_cast<GLfloat*>((*uni)->value));
			}
				break;
			case U_VEC2:
			{
				glUniform2fv(uni_location, 1, reinterpret_cast<GLfloat*>((*uni)->value));
			}
				break;
			case U_VEC3:
			{
				glUniform3fv(uni_location, 1, reinterpret_cast<GLfloat*>((*uni)->value));
			}
				break;
			case U_VEC4:
			{
				glUniform4fv(uni_location, 1, reinterpret_cast<GLfloat*>((*uni)->value));
			}
			break;
			case U_MAT4X4:
			{
				glUniformMatrix4fv(uni_location, 1, GL_FALSE, reinterpret_cast<GLfloat*>((*uni)->value));
			}
				break;
			case U_SAMPLER2D:
				//Already handled before.
				break;
			}
		}
	}

	//Time(special)
	GLint time_location = glGetUniformLocation(shader_id, "time");
	if (time_location != -1)
	{
		glUniform1f(time_location, time->GetUnitaryTime());
	}

	//Buffer vertices == 0
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, obj->mesh_to_draw->id_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	//Buffer uvs == 1
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, obj->mesh_to_draw->id_uvs);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	//Buffer normals == 2
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, obj->mesh_to_draw->id_normals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	//Buffer tangents == 3
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, obj->mesh_to_draw->id_tangents);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	//Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->mesh_to_draw->id_indices);
	glDrawElements(GL_TRIANGLES, obj->mesh_to_draw->num_indices, GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void ModuleRenderer3D::SetClearColor(const math::float3 & color) const
{
	glClearColor(color.x, color.y, color.z, 1.0f);
}

void ModuleRenderer3D::RemoveBuffer(unsigned int id)
{
	//Patch for issue (https://github.com/traguill/Ezwix-Engine/issues/13). TODO: Solve the issue!
	if(id != 9)
		glDeleteBuffers(1, (GLuint*)&id);
}
