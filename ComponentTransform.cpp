#include "ComponentTransform.h"
#include "GameObject.h"
#include "Globals.h"
#include "imgui\imgui.h"
#include "ImGuizmo\ImGuizmo.h"
#include "ComponentMesh.h"
#include "Data.h"

//CHANGE - PEP
//Not sure to include this here, maybe better to create a new file
#include "Application.h"
#include "ModuleCamera3D.h"
#include "ComponentCamera.h"

ComponentTransform::ComponentTransform(ComponentType type, GameObject* game_object, math::float4x4** global_matrix) : Component(type, game_object)
{
	CalculateFinalTransform();
	*global_matrix = &final_transform_matrix;

}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::Update()
{
	if (transform_modified)
	{
		transform_matrix = transform_matrix.FromTRS(position, rotation, scale);
		CalculateFinalTransform();

		transform_modified = false;
	}
}

void ComponentTransform::OnInspector()
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		ImVec4 white = ImVec4(1, 1, 1, 1);

		//CHANGE - PEP
		//---------Transform operation
		ImGuizmo::BeginFrame();

		//Selection keys
		if (ImGui::IsKeyPressed(90))
			guizmo_op = TRANSLATE;
		if (ImGui::IsKeyPressed(69))
			guizmo_op = ROTATION;
		if (ImGui::IsKeyPressed(82))
			guizmo_op = SCALE;

		//Inspector selection
		if (ImGui::RadioButton("Translate", guizmo_op == TRANSLATE))
		{
			guizmo_op = TRANSLATE;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Rotate", guizmo_op == ROTATION))
		{
			guizmo_op = ROTATION;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Scale", guizmo_op == SCALE))
		{
			guizmo_op = SCALE;
		}

		ImGui::Checkbox("Enable Gizmo", &guizmo_enable);

		

		SetGizmo();
		//
	
		//Position
		ImGui::TextColored(white, "Position: ");
		ImGui::SameLine();

		float3 position = this->position;
		if (ImGui::DragFloat3("##pos", position.ptr()))
		{
			SetPosition(position);
		}

		//Rotation
		ImGui::TextColored(white, "Rotation: ");
		ImGui::SameLine();

		float3 rotation = this->rotation_euler;
		if (ImGui::DragFloat3("##rot", rotation_euler.ptr(), 1.0f, -360.0f, 360.0f))
		{
			SetRotation(rotation_euler);
		}

		//Scale
		ImGui::TextColored(white, "Scale: ");
		ImGui::SameLine();

		float3 scale = this->scale;
		if (ImGui::DragFloat3("##scale", scale.ptr()))
		{
			SetScale(scale);
		}

		//Local Matrix
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[0][0], transform_matrix.v[0][1], transform_matrix.v[0][2], transform_matrix.v[0][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[1][0], transform_matrix.v[1][1], transform_matrix.v[1][2], transform_matrix.v[1][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[2][0], transform_matrix.v[2][1], transform_matrix.v[2][2], transform_matrix.v[2][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[3][0], transform_matrix.v[3][1], transform_matrix.v[3][2], transform_matrix.v[3][3]);

		//Global Matrix
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", final_transform_matrix.v[0][0], final_transform_matrix.v[0][1], final_transform_matrix.v[0][2], final_transform_matrix.v[0][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", final_transform_matrix.v[1][0], final_transform_matrix.v[1][1], final_transform_matrix.v[1][2], final_transform_matrix.v[1][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", final_transform_matrix.v[2][0], final_transform_matrix.v[2][1], final_transform_matrix.v[2][2], final_transform_matrix.v[2][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", final_transform_matrix.v[3][0], final_transform_matrix.v[3][1], final_transform_matrix.v[3][2], final_transform_matrix.v[3][3]);
	}

	
	
}

void ComponentTransform::SetPosition(const math::float3& pos)
{
	position = pos;

	transform_modified = true;
}

void ComponentTransform::SetRotation(const math::float3& rot_euler)
{
	rotation_euler = rot_euler;
	
	float3 rot_deg = DegToRad(rot_euler);

	rotation = rotation.FromEulerXYZ(rot_deg.x, rot_deg.y, rot_deg.z);

	transform_modified = true;
}

void ComponentTransform::SetRotation(const math::Quat& rot)
{
	rotation = rot;

	rotation_euler = RadToDeg(rotation.ToEulerXYZ());

	transform_modified = true;
}

void ComponentTransform::SetScale(const math::float3& scale)
{
	this->scale = scale;

	transform_modified = true;
}

//CHANGE - PEP
void ComponentTransform::SetGizmo()
{
	ImGuizmo::Enable(guizmo_enable);

	ComponentCamera* cam = App->camera->GetEditorCamera();
	float4x4 guizmo_matrix = final_transform_matrix;
	float4x4 manipulated_matrix = manipulated_matrix.identity;

	//ImGuizmo::DrawCube(cam->GetViewMatrix().ptr(), cam->GetProjectionMatrix().ptr(), guizmo_matrix.ptr());
	//ImGuizmo::DrawCube(cam->GetViewMatrix().ptr(), cam->GetProjectionMatrix().ptr(), manipulated_matrix.ptr());

	ImGuizmo::Manipulate(cam->GetViewMatrix().ptr(), 
						cam->GetProjectionMatrix().ptr(),
						(ImGuizmo::OPERATION)guizmo_op, 
						ImGuizmo::WORLD, 
						transform_matrix.ptr(),
						manipulated_matrix.ptr(),
						NULL);
	
	//ImGuizmo::DrawCube(cam->GetViewMatrix().ptr(), cam->GetProjectionMatrix().ptr(), final_transform_matrix.ptr());
	
	if (ImGuizmo::IsUsing())
	{
	
		//transform_matrix = guizmo_matrix;
		//transform_matrix += manipulated_matrix;

		//transform_matrix.Decompose(position, rotation, scale);
		//float3 d_position, d_rotation, d_scale;
		ImGuizmo::DecomposeMatrixToComponents(manipulated_matrix.ptr(), position.ptr(), rotation.ptr(), scale.ptr());
		SetPosition(position);
		SetRotation(rotation);
		SetScale(scale);
		
	
	}

	
}

math::float3 ComponentTransform::GetPosition() const
{
	return position;
}

math::float3 ComponentTransform::GetRotationEuler() const
{
	return rotation_euler;
}

math::Quat ComponentTransform::GetRotation() const
{
	return rotation;
}

math::float3 ComponentTransform::GetScale()const
{
	return scale;
}

math::float4x4 ComponentTransform::GetTransformMatrix()const
{
	return final_transform_matrix.Transposed();
}

math::float4x4 ComponentTransform::GetGlobalMatrix()const
{
	return final_transform_matrix;
}

void ComponentTransform::Save(Data & file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendMatrix("matrix", transform_matrix);

	file.AppendArrayValue(data);
}

void ComponentTransform::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	transform_matrix = conf.GetMatrix("matrix");

	position = transform_matrix.TranslatePart();
	rotation_euler = transform_matrix.ToEulerXYZ(); //In radians for now.
	rotation = Quat::FromEulerXYZ(rotation_euler.x, rotation_euler.y, rotation_euler.z); 
	rotation_euler = RadToDeg(rotation_euler); //To degrees
	scale = transform_matrix.GetScale();
	CalculateFinalTransform();
}

void ComponentTransform::Remove()
{
	LOG("Component Transform from GO(%s) can't be removed.",GetGameObject()->name);
}

void ComponentTransform::CalculateFinalTransform()
{
	GameObject* game_object = GetGameObject();
	if (game_object)
	{
		if (game_object->GetParent())
		{
			ComponentTransform* parent_transform = (ComponentTransform*)game_object->GetParent()->GetComponent(C_TRANSFORM);
			assert(parent_transform);

			final_transform_matrix = parent_transform->final_transform_matrix * transform_matrix;

			std::vector<GameObject*>::const_iterator go_childs = game_object->GetChilds()->begin();
			for (go_childs; go_childs != game_object->GetChilds()->end(); ++go_childs)
			{
				ComponentTransform* transform = (ComponentTransform*)(*go_childs)->GetComponent(C_TRANSFORM);
				if (transform)
				{
					transform->CalculateFinalTransform();
				}
			}
		}
		else
		{
			final_transform_matrix = transform_matrix;
		}	

		game_object->TransformModified();
	}
	else
	{	
		LOG("Error: Component created but not attached to any gameobject");
	}
}

void ComponentTransform::Rotate(const math::Quat & quaternion)
{
	rotation = quaternion * rotation;
	transform_modified = true;
}
