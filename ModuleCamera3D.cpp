#include "ModuleCamera3D.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"

#include "ComponentCamera.h"

#include "Time.h"

#include "SDL/include/SDL_scancode.h"
#include "SDL/include/SDL_mouse.h"

ModuleCamera3D::ModuleCamera3D(const char* name, bool start_enabled) : Module(name, start_enabled)
{ }

ModuleCamera3D::~ModuleCamera3D()
{ }

bool ModuleCamera3D::Init(Data & config)
{
	camera = new ComponentCamera(C_CAMERA, nullptr);
	RemoveSceneCamera(camera); //Editor cam is not a scene camera
	camera->frustum.SetPos(vec(0, 3, -10));
	camera->viewport_rel_position.x = 0.0f;
	camera->viewport_rel_position.y = 0.0f;
	camera->viewport_rel_size.x = 1.0f;
	camera->viewport_rel_size.y = 1.0f;

	reference = float3(0, 0, 0);
	camera->LookAt(reference);	

	App->renderer3D->SetCamera(camera);

	// Camera acts as the audio listener
	//App->audio->SetListener(camera);

	return true;
}

bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;

	return ret;
}

bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");

	delete camera; //Note: components are destroyed inside the destructor of the GameObject

	return true;
}

void ModuleCamera3D::OnPlay()
{
	//Removing current camera
	App->renderer3D->SetCamera(nullptr);

	/*if (scene_cameras.size() > 0)
		App->audio->SetListener(*scene_cameras.begin());*/

	for (vector<ComponentCamera*>::iterator it = scene_cameras.begin(); it != scene_cameras.end(); ++it)
	{
		App->renderer3D->AddCamera((*it));	
	}
}

void ModuleCamera3D::OnStop()
{
	App->renderer3D->SetCamera(GetEditorCamera());
}

update_status ModuleCamera3D::Update()
{
	EditorCameraMovement(time->RealDeltaTime());

	return UPDATE_CONTINUE;
}

math::float3 ModuleCamera3D::GetPosition() const
{
	return camera->frustum.Pos();
}

math::float4x4 ModuleCamera3D::GetViewMatrix() const
{
	return camera->GetViewMatrix();
}

float ModuleCamera3D::GetNearPlane() const
{
	return camera->GetNearPlane();
}

float ModuleCamera3D::GetFarPlane() const
{
	return camera->GetFarPlane();
}

float ModuleCamera3D::GetFOV() const
{
	return camera->GetFOV();
}

float ModuleCamera3D::GetAspectRatio() const
{
	return camera->aspect_ratio;
}

void ModuleCamera3D::SetNearPlane(const float & near_plane)
{
	camera->SetNearPlane(near_plane);
}

void ModuleCamera3D::SetFarPlane(const float & far_plane)
{
	camera->SetFarPlane(far_plane);
}

void ModuleCamera3D::SetFOV(const float & fov)
{
	camera->SetFOV(fov);
}

void ModuleCamera3D::SetBackgroundColor(const math::float3 & color)
{
	camera->SetBackgroundColor(color);
	App->renderer3D->SetClearColor(color);
}

void ModuleCamera3D::SetAspectRatio(float ar)
{
	camera->SetAspectRatio(ar);
}

bool ModuleCamera3D::MoveArrows(float dt)
{	
	float speed = 8.0f * dt;

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
		speed *= 2;

	//Arrows movement
	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	{
		reference += camera->frustum.Up() * speed;
		camera->frustum.SetPos(camera->frustum.Pos() + camera->frustum.Up() * speed);
	}
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	{
		reference -= camera->frustum.Up() * speed;
		camera->frustum.SetPos(camera->frustum.Pos() - camera->frustum.Up() * speed);
	}
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT) 
	{
		reference -= camera->frustum.WorldRight() * speed;
		camera->frustum.SetPos(camera->frustum.Pos() - camera->frustum.WorldRight() * speed);
	}
	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	{
		reference += camera->frustum.WorldRight() * speed;
		camera->frustum.SetPos(camera->frustum.Pos() + camera->frustum.WorldRight() * speed);
	}
	
	return false;
}

bool ModuleCamera3D::MoveMouse(float dt)
{
	int motion_x, motion_y;
	bool ret = false;
	motion_x = -App->input->GetMouseXMotion();
	motion_y = App->input->GetMouseYMotion();

	//Orbit around reference
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT && (motion_x != 0 || motion_y != 0))
	{
		Orbit(motion_x, -motion_y);
		ret = true;
	}
	
	//Middle button movement
	if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT && (motion_x != 0 || motion_y != 0))
	{
		//TODO: Kind of magic number. Consider other options?
		float distance = reference.Distance(GetPosition());
		float3 Y_add = camera->GetUp() * motion_y * (distance / 1000);
		float3 X_add = camera->GetWorldRight() * motion_x * (distance / 1000);

		reference += X_add;
		reference += Y_add;

		camera->frustum.SetPos(GetPosition() + X_add + Y_add);
	}

	//Zoom
	int wheel = App->input->GetMouseZ();
	if (wheel != 0)
	{
		Zoom(wheel);
		ret = true;
	}
	return ret;
}

void ModuleCamera3D::Orbit(int x, int y)
{
	float3 vector = GetPosition() - reference;

	Quat quat_y(camera->GetUp(), x * 0.003);
	Quat quat_x(camera->GetWorldRight(), y * 0.003);

	vector = quat_x.Transform(vector);
	vector = quat_y.Transform(vector);

	camera->frustum.SetPos(vector + reference);
	camera->LookAt(reference);
}

void ModuleCamera3D::Zoom(float value)
{
	float distance = reference.Distance(GetPosition());
	float3 front = camera->GetFront();
	float3 position = camera->frustum.Pos();
	vec newPos = GetPosition() + camera->GetFront() * value *distance * 0.05f;
	camera->frustum.SetPos(newPos);
}

//Keeping same distance as reference by now
void ModuleCamera3D::Center(const float3& position)
{
	float distance = reference.Distance(camera->frustum.Pos());
	camera->Center(position, distance);
	reference = position;
}

math::float3 ModuleCamera3D::GetBackgroundColor() const
{
	return camera->GetBackgroundColor();
}

ComponentCamera * ModuleCamera3D::GetEditorCamera() const
{
	return camera;
}

void ModuleCamera3D::AddSceneCamera(ComponentCamera * cam)
{
	scene_cameras.push_back(cam);
}

void ModuleCamera3D::RemoveSceneCamera(ComponentCamera * cam)
{
	vector<ComponentCamera*>::iterator f_cam = std::find(scene_cameras.begin(), scene_cameras.end(), cam);
	if(f_cam != scene_cameras.end())
		scene_cameras.erase(f_cam);
}

void ModuleCamera3D::EditorCameraMovement(float dt)
{
	MoveArrows(dt);
	MoveMouse(dt);
}



