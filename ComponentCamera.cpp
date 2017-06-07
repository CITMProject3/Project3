#include "Application.h"
#include "ModulePhysics3D.h"
#include "ComponentCamera.h"
#include "imgui\imgui.h"
#include "DebugDraw.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "LayerSystem.h"
#include "ResourceFileRenderTexture.h"
#include "Assets.h"

#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleGOManager.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"

#include "Brofiler\include\Brofiler.h"

ComponentCamera::ComponentCamera(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	//Init frustrum
	aspect_ratio = (float)App->window->GetScreenWidth() / (float)App->window->GetScreenHeight();
	float vertical_fov = DegToRad(fov);
	float horizontal_fov = 2.0f*atanf(tanf(vertical_fov / 2.0f) * aspect_ratio);

	frustum.SetPerspective(horizontal_fov, vertical_fov);
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);

	if (game_object)
	{
		float4x4 matrix = game_object->GetGlobalMatrix();
		frustum.SetPos(matrix.TranslatePart());
		frustum.SetFront(matrix.WorldZ());
		frustum.SetUp(matrix.WorldY());
	}
	else
	{
		frustum.SetPos(float3(0, 0, 0));
		frustum.SetFront(float3::unitZ);
		frustum.SetUp(float3::unitY);
	}

	frustum.SetViewPlaneDistances(near_plane, far_plane);
	frustum.SetVerticalFovAndAspectRatio(DegToRad(fov), aspect_ratio);

	color = float3(0, 0, 0); //Black to clear the screen by default

	OnTransformModified();
	App->renderer3D->AddObserver(this);

	App->camera->AddSceneCamera(this);
}

ComponentCamera::~ComponentCamera()
{
	App->renderer3D->RemoveObserver(this);
	if(render_texture)
		render_texture->Unload();
	App->camera->RemoveSceneCamera(this);
}

void ComponentCamera::PreUpdate()
{
	UpdateCameraFrustum();
}

void ComponentCamera::Update()
{
	if (App->StartInGame() == false && App->IsGameRunning() == false && App->physics->renderColliders)
			g_Debug->AddFrustum(frustum, 30.0f, g_Debug->blue, 2.0f);
}

void ComponentCamera::OnInspector(bool debug)
{
	string str = (string("Camera") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##camera");
		}

		if (ImGui::BeginPopup("delete##camera"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		ImGui::Checkbox("Render terrain", &renderTerrain);
		ImGui::Checkbox("Smooth follow", &smoothFollow);
		if (smoothFollow)
		{
			ImGui::Text("Position follow speed:");
			ImGui::DragFloat("##smoothFollowPos", &followMoveSpeed, 0.01f, 0.01f, 0.99f);
			ImGui::Text("Rotation follow speed:");
			ImGui::DragFloat("##smoothFollowRot", &followRotateSpeed, 0.01f, 0.01f, 0.99f);
		}
		ImGui::Separator();
		ImGui::Text("Viewport (relative to screen)");
		//ImGui::PushStyleColor(ImGuiCol_Text)
		ImGui::Text("0 to 1 values");
		if (ImGui::InputFloat("Position x", &viewport_rel_position.x, 0.0f, 1.0f))
		{
			math::Clamp(viewport_rel_position.x, 0.0f, 1.0f);
			UpdateViewportDimensions();
		}
		if (ImGui::InputFloat("Position y", &viewport_rel_position.y, 0.0f, 1.0f))
		{
			math::Clamp(viewport_rel_position.y, 0.0f, 1.0f);
			UpdateViewportDimensions();
		}
		if (ImGui::InputFloat("Size x ", &viewport_rel_size.x, 0.0f, 1.0f))
		{
			math::Clamp(viewport_rel_size.x, 0.0f, 1.0f);
			UpdateViewportDimensions();
		}
		if (ImGui::InputFloat("Size y", &viewport_rel_size.y, 0.0f, 1.0f))
		{
			math::Clamp(viewport_rel_size.y, 0.0f, 1.0f);
			UpdateViewportDimensions();
		}


		ImGui::Separator();
		ImGui::Text("Frustum");
		//Near plane
		ImGui::Text("Near Plane: ");
		float near_value = near_plane;
		if (ImGui::SliderFloat("##near_p", &near_value, 0, 1, "%.3f", 0.05f))
			SetNearPlane(near_value);

		//Far plane
		ImGui::Text("Far Plane: ");
		float far_value = far_plane;
		if (ImGui::SliderFloat("##far_p", &far_value, 0, 2000))
			SetFarPlane(far_value);

		//Field of view
		ImGui::Text("Field of view: ");
		float fov_value = fov;
		if (ImGui::SliderFloat("##fov", &fov_value, 0, 180))
			SetFOV(fov_value);

		ImGui::Text("Background color: "); ImGui::SameLine();
		float3 color = this->color;
		if (ImGui::ColorEdit3("", color.ptr()))
		{
			this->color = color;
		}

		//LayerMask
		App->go_manager->layer_system->DisplayLayerMask(layer_mask);

		//RenderTexture
		string ren_name = "RenderTexture: " + ((render_texture) ? render_texture_path : "none");
		if (ImGui::BeginMenu(ren_name.data()))
		{
			vector<string> rentex_list;
			App->editor->assets->GetAllFilesByType(FileType::RENDER_TEXTURE, rentex_list);

			for (vector<string>::iterator rentex = rentex_list.begin(); rentex != rentex_list.end(); rentex++)
			{
				if (ImGui::MenuItem((*rentex).data()))
				{
					render_texture_path = (*rentex).data();
					render_texture_path_lib = App->resource_manager->FindFile(render_texture_path);
					render_texture = (ResourceFileRenderTexture*)App->resource_manager->LoadResource(render_texture_path_lib, ResourceFileType::RES_RENDER_TEX);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::Checkbox("Render skybox###render_skybox_cam", &render_skybox);

		ImGui::Separator();

		if (ImGui::Button("Remove ###cam_rem"))
		{
			Remove();
		}
	}		
}

void ComponentCamera::OnTransformModified()
{
	GameObject* game_object = GetGameObject();

	if (game_object)
		desiredTransform = game_object->transform->GetGlobalMatrix();
	else
	{
		LOG("[ERROR] Component Camera is trying to update it's matrix but it is not attached to any game object.");
		App->editor->DisplayWarning(WarningType::W_ERROR, "Component Camera is trying to update it's matrix but it is not attached to any game object" );
	}
}

void ComponentCamera::OnNotify(void * entity, Event event)
{
	if (event == Event::WINDOW_RESIZE)
	{
		UpdateViewportDimensions();
	}
}

void ComponentCamera::UpdateViewportDimensions()
{
	viewport_position.x = (float)App->window->GetScreenWidth() * viewport_rel_position.x;
	viewport_position.y = (float)App->window->GetScreenHeight() * viewport_rel_position.y;

	viewport_size.x = (float)App->window->GetScreenWidth() * viewport_rel_size.x;
	viewport_size.y = (float)App->window->GetScreenHeight() * viewport_rel_size.y;

	aspect_ratio = viewport_size.x / viewport_size.y;
	frustum.SetVerticalFovAndAspectRatio(DegToRad(fov), aspect_ratio);
}

float ComponentCamera::GetNearPlane() const
{
	return near_plane;
}

float ComponentCamera::GetFarPlane() const
{
	return far_plane;
}

float ComponentCamera::GetFOV() const
{
	return fov;
}

math::float3 ComponentCamera::GetFront() const
{
	return frustum.Front();
}

math::float3 ComponentCamera::GetUp() const
{
	return frustum.Up();
}

math::float3 ComponentCamera::GetPos()const
{
	return frustum.Pos();
}

math::float4x4 ComponentCamera::GetProjectionMatrix() const
{
	math::float4x4 matrix = frustum.ProjectionMatrix();
	matrix.Transpose();
	return matrix;
}

math::float3 ComponentCamera::GetBackgroundColor() const
{
	return color;
}

math::float3 ComponentCamera::GetWorldRight() const
{
	return frustum.WorldRight();
}

void ComponentCamera::SetNearPlane(float value)
{
	if (value < far_plane && value > 0.0f)
		near_plane = value;
	frustum.SetViewPlaneDistances(near_plane, far_plane);

	properties_modified = true;
}

void ComponentCamera::SetFarPlane(float value)
{
	if (value > near_plane)
		far_plane = value;

	frustum.SetViewPlaneDistances(near_plane, far_plane);

	properties_modified = true;
}

void ComponentCamera::SetFOV(float value)
{
	//TODO: check for min -max values of fov
	fov = value;

	frustum.SetVerticalFovAndAspectRatio(DegToRad(fov), aspect_ratio);

	properties_modified = true;
}

void ComponentCamera::SetAspectRatio(float value)
{
	aspect_ratio = value;
	SetFOV(fov);
	//float horizontalFov = frustum.HorizontalFov();
	//frustum.SetHorizontalFovAndAspectRatio(horizontalFov, value);
	//properties_modified = true;
}

void ComponentCamera::LookAt(const math::float3 & point)
{
	float3 look_direction = point - frustum.Pos();

	float3x3 matrix = float3x3::LookAt(frustum.Front(), look_direction.Normalized(), frustum.Up(), float3::unitY);

	frustum.SetFront(matrix.MulDir(frustum.Front()).Normalized());
	frustum.SetUp(matrix.MulDir(frustum.Up()).Normalized());

	properties_modified = true;
}

void ComponentCamera::Center(const float3& position, float distance)
{
	float3 v = frustum.Front().Neg();
	frustum.SetPos(position + v * distance);
}

void ComponentCamera::SetBackgroundColor(const math::float3 color)
{
	this->color = color;
}

bool ComponentCamera::Intersects(const math::AABB & box)const
{
	bool ret = true;
	math::vec corners[8];
	box.GetCornerPoints(corners);

	math::Plane planes[6];
	frustum.GetPlanes(planes);

	for (int p = 0; p < 6; p++)
	{
		int count = 0;
		for (int i = 0; i < 8; i++)
		{
			if (planes[p].IsOnPositiveSide(corners[i]))
				count++;
		}

		if (count == 8)
		{
			ret = false;
			break;
		}
	}

	return ret;
}

math::float4x4 ComponentCamera::GetViewMatrix() const
{
	math::float4x4 matrix = frustum.ViewMatrix();
	return matrix.Transposed();
}

math::float4x4 ComponentCamera::GetWorldMatrix() const
{
	return frustum.WorldMatrix();
}

int ComponentCamera::GetLayerMask() const
{
	return layer_mask;
}

void ComponentCamera::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	
	data.AppendFloat("near_plane", near_plane);
	data.AppendFloat("far_plane", far_plane);
	data.AppendFloat("fov", fov);
	data.AppendFloat("aspect_ratio", aspect_ratio);
	data.AppendFloat3("color", color.ptr());
	data.AppendInt("layer_mask", layer_mask);
	data.AppendString("render_texture_path", render_texture_path.data());
	data.AppendString("render_texture_path_lib", render_texture_path_lib.data());

	data.AppendFloat("viewport_rel_pos_x", viewport_rel_position.x);
	data.AppendFloat("viewport_rel_pos_y", viewport_rel_position.y);

	data.AppendFloat("viewport_rel_size_x", viewport_rel_size.x);
	data.AppendFloat("viewport_rel_size_y", viewport_rel_size.y);

	data.AppendBool("followSmooth", smoothFollow);
	data.AppendFloat("followMovSpeed", followMoveSpeed);
	data.AppendFloat("followRotSpeed", followRotateSpeed);

	data.AppendBool("render_terrain", renderTerrain);
	data.AppendBool("render_skybox", render_skybox);

	file.AppendArrayValue(data);
}

void ComponentCamera::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	near_plane = conf.GetFloat("near_plane");
	far_plane = conf.GetFloat("far_plane");
	fov = conf.GetFloat("fov");
	aspect_ratio = conf.GetFloat("aspect_ratio");
	color = conf.GetFloat3("color");
	layer_mask = conf.GetInt("layer_mask");
	render_texture_path = conf.GetString("render_texture_path");
	render_texture_path_lib = conf.GetString("render_texture_path_lib");

	viewport_rel_position.x = conf.GetFloat("viewport_rel_pos_x");
	viewport_rel_position.y = conf.GetFloat("viewport_rel_pos_y");

	viewport_rel_size.x = conf.GetFloat("viewport_rel_size_x");
	viewport_rel_size.y = conf.GetFloat("viewport_rel_size_y");

	smoothFollow = conf.GetBool("followSmooth");
	followMoveSpeed = conf.GetFloat("followMovSpeed");
	followRotateSpeed = conf.GetFloat("followRotSpeed");

	renderTerrain = conf.GetBool("render_terrain");
	render_skybox = conf.GetBool("render_skybox");

	//Init frustrum
	float vertical_fov = DegToRad(fov);
	float horizontal_fov = 2.0f*atanf(tanf(vertical_fov / 2.0f) * aspect_ratio);

	frustum.SetPerspective(horizontal_fov, vertical_fov);
	frustum.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	frustum.SetPos(float3::zero);
	frustum.SetFront(float3::unitZ);
	frustum.SetUp(float3::unitY);
	frustum.SetViewPlaneDistances(near_plane, far_plane);
	frustum.SetVerticalFovAndAspectRatio(DegToRad(fov), aspect_ratio);
	UpdateViewportDimensions();

	OnTransformModified();

	//Init render texture
	if (render_texture_path_lib.size() != 0)
		render_texture = (ResourceFileRenderTexture*)App->resource_manager->LoadResource(render_texture_path_lib, ResourceFileType::RES_RENDER_TEX);
}

math::Ray ComponentCamera::CastCameraRay(math::float2 screen_pos)
{
	float2 pos = screen_pos;

	pos.x = 2.0f * pos.x / (float)App->window->GetScreenWidth() - 1.0f;
	pos.y = 1.0f - 2.0f * pos.y / (float)App->window->GetScreenHeight();
	
	Ray ray = frustum.UnProjectFromNearPlane(pos.x, pos.y);
 
	return ray;
}

void ComponentCamera::UpdateCameraFrustum()
{
	BROFILER_CATEGORY("ComponentCamera::UpdateCameraFrustum", Profiler::Color::MediumTurquoise)
	if (smoothFollow == true)
	{		
		float3 curr_pos, des_pos, scale;
		Quat curr_rot, des_rot;
		desiredTransform.Decompose(des_pos, des_rot, scale);
		currentTransform.Decompose(curr_pos, curr_rot, scale);

	
		Quat rotation = curr_rot.Lerp(des_rot, followRotateSpeed);
		float3 position = curr_pos.Lerp(des_pos, followMoveSpeed);
		currentTransform = float4x4::FromTRS(position, rotation, float3::one);
	}
	else
	{
		currentTransform = desiredTransform;
	}
	frustum.SetPos(currentTransform.TranslatePart());
	frustum.SetFront(currentTransform.WorldZ());
	frustum.SetUp(currentTransform.WorldY());
}
