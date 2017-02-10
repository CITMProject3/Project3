#include "Application.h"
#include "ComponentCamera.h"
#include "imgui\imgui.h"
#include "DebugDraw.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "LayerSystem.h"
#include "ResourceFileRenderTexture.h"
#include "Assets.h"

ComponentCamera::ComponentCamera(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	//Init frustrum
	aspect_ratio = (float)App->window->GetScreenWidth() / (float)App->window->GetScreenHeight();
	float vertical_fov = DegToRad(fov);
	float horizontal_fov = 2.0f*atanf(tanf(vertical_fov / 2.0f) * aspect_ratio);

	frustum.SetPerspective(horizontal_fov, vertical_fov);
	frustum.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	float4x4 matrix = game_object->GetGlobalMatrix();
	frustum.SetPos(matrix.TranslatePart());
	frustum.SetFront(matrix.WorldZ());
	frustum.SetUp(matrix.WorldY());
	frustum.SetViewPlaneDistances(near_plane, far_plane);
	frustum.SetVerticalFovAndAspectRatio(DegToRad(fov), aspect_ratio);

	color = float3(0, 0, 0); //Black to clear the screen by default

	App->renderer3D->AddObserver(this);

}

ComponentCamera::~ComponentCamera()
{
	App->renderer3D->RemoveObserver(this);
	if(render_texture)
		render_texture->Unload();
}

void ComponentCamera::Update()
{
	g_Debug->AddFrustum(frustum, 30.0f, g_Debug->blue, 2.0f);
}

void ComponentCamera::OnInspector()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
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

		ImGui::Text("Aspect ratio: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 0, 1, 1), "&0.2f", aspect_ratio);

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
	{
		ComponentTransform* trans = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		
		float4x4 matrix = trans->GetGlobalMatrix();

		frustum.SetPos(matrix.TranslatePart());
		frustum.SetFront(matrix.WorldZ());
		frustum.SetUp(matrix.WorldY());
	}
	else
		LOG("Error: Component Camera is trying to update it's matrix but it is not attached to any game object.");

}

void ComponentCamera::OnNotify(void * entity, Event event)
{
	if (event == Event::WINDOW_RESIZE)
	{
		aspect_ratio = (float)App->window->GetScreenWidth() / (float)App->window->GetScreenHeight();
		frustum.SetVerticalFovAndAspectRatio(DegToRad(fov), aspect_ratio);
	}
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

void ComponentCamera::LookAt(const math::float3 & point)
{
	math::float3 look_direction = point - frustum.Pos();
	math::float3x3 matrix = math::float3x3::LookAt(frustum.Front(), look_direction, frustum.Up(), math::float3::unitY);

	frustum.SetFront(matrix.MulDir(frustum.Front()).Normalized());
	frustum.SetFront(matrix.MulDir(frustum.Up()).Normalized());
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
