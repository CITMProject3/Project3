#include "Application.h"
#include "CameraWindow.h"
#include "ModuleCamera3D.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "ModuleGOManager.h"

CameraWindow::CameraWindow()
{}

CameraWindow::~CameraWindow()
{}

void CameraWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Camera Options", &active);

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
	
	ImGui::Text("Background color: "); ImGui::SameLine();
	float3 color = App->camera->GetBackgroundColor();
	if (ImGui::ColorEdit3("", color.ptr()))
	{
		App->camera->SetBackgroundColor(color);
	}

	ImGui::Text("Current camera: ");
	ImGui::SameLine();

	if (ImGui::BeginMenu(App->camera->GetCurrentCamera()->GetGameObject()->name.data()))
	{
		vector<ComponentCamera*> cameras;
		App->go_manager->GetAllCameras(cameras);

		ComponentCamera* cam_editor = App->camera->GetEditorCamera();
		//Camera editor as option too
		if (ImGui::MenuItem(cam_editor->GetGameObject()->name.data()))
		{
			App->camera->ChangeCurrentCamera(cam_editor);
		}

		for (vector<ComponentCamera*>::iterator cam_it = cameras.begin(); cam_it != cameras.end(); cam_it++)
			if (ImGui::MenuItem((*cam_it)->GetGameObject()->name.data()))
			{
				App->camera->ChangeCurrentCamera((*cam_it));
			}

		ImGui::EndMenu();
	}

	ImGui::End();
}

