#include "Application.h"
#include "CameraWindow.h"
#include "ModuleCamera3D.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "ModuleGOManager.h"

#include "ModuleRenderer3D.h"

CameraWindow::CameraWindow()
{

}

CameraWindow::~CameraWindow()
{
}

void CameraWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Camera Options", &active, flags);

	//Near plane
	ImGui::Text("Near Plane: ");
	float near_value = App->camera->GetNearPlane();
	if (ImGui::SliderFloat("##near_p", &near_value, 0, 1, "%.3f", 0.05f))
		App->camera->SetNearPlane(near_value);

	//Far plane
	ImGui::Text("Far Plane: ");
	float far_value = App->camera->GetFarPlane();
	if (ImGui::SliderFloat("##far_p", &far_value, 0, 2000))
		App->camera->SetFarPlane(far_value);

	//Field of view
	ImGui::Text("Field of view: ");
	float fov_value = App->camera->GetFOV();
	if (ImGui::SliderFloat("##fov", &fov_value, 0, 180))
		App->camera->SetFOV(fov_value);
	
	ImGui::Text("Aspect Ratio: ");
	float aspect_ratio = App->camera->GetAspectRatio();
	if (ImGui::SliderFloat("##ar", &aspect_ratio, 0, 5))
		App->camera->SetAspectRatio(aspect_ratio);

	ImGui::Text("Background color: "); ImGui::SameLine();
	float3 color = App->camera->GetBackgroundColor();
	if (ImGui::ColorEdit3("", color.ptr()))
	{
		App->camera->SetBackgroundColor(color);
	}

	ImGui::Text("Current camera: ");
	ImGui::SameLine();


	ComponentCamera* current_camera = App->renderer3D->cameras[0];
	std::string text = "Editor Camera";
	if (current_camera != nullptr)
	{
		if (current_camera->GetGameObject() != nullptr)
		{
			text = current_camera->GetGameObject()->name.c_str();
		}
	}

	if (ImGui::BeginMenu(text.c_str()))
	{
		vector<Component*> cameras;
		App->go_manager->GetAllComponents(cameras, ComponentType::C_CAMERA);

		//Camera editor as option too
		if (ImGui::MenuItem("Editor Camera"))
		{
			App->renderer3D->SetCamera(App->camera->GetEditorCamera());
		}

		for (vector<Component*>::iterator cam_it = cameras.begin(); cam_it != cameras.end(); cam_it++)
			if (ImGui::MenuItem((*cam_it)->GetGameObject()->name.data()))
			{
				App->renderer3D->SetCamera((ComponentCamera*)(*cam_it));
			}

		ImGui::EndMenu();
	}

	ImGui::End();
}

