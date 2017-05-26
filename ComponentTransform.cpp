#include "Application.h"
#include "ModuleInput.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"

#include "Globals.h"
#include "Data.h"

#include "imgui\imgui.h"
#include "ImGuizmo\ImGuizmo.h"

#include "Brofiler\include\Brofiler.h"

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
	BROFILER_CATEGORY("ComponentTransform::Update", Profiler::Color::Tomato)

	UpdateMatrix();
}

void ComponentTransform::OnInspector(bool debug)
{
	string str = (string("Transform") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("reset##transform");
		}

		if (ImGui::BeginPopup("reset##transform"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				Reset();
			}
			ImGui::EndPopup();
		}

		ImVec4 white = ImVec4(1, 1, 1, 1);

		//CHANGE - PEP
		//---------Transform operation
		ImGuizmo::BeginFrame();

		//Inspector selection
		if (ImGui::RadioButton("Translate", App->editor->gizmo_operation == TRANSLATE))
		{
			App->editor->gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Rotate", App->editor->gizmo_operation == ROTATION))
		{
			App->editor->gizmo_operation = ImGuizmo::OPERATION::ROTATE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", App->editor->gizmo_operation == SCALE))
		{
			App->editor->gizmo_operation = ImGuizmo::OPERATION::SCALE;
		}	

		ImGui::Checkbox("Enable Gizmo", &App->editor->gizmo_enabled);

		//Position
		ImGui::TextColored(white, "Position: ");
		ImGui::SameLine();

		float3 position = this->position;
		if (ImGui::DragFloat3("##pos", position.ptr()))
		{
			App->input->InfiniteHorizontal();
			SetPosition(position);
		}

		//Rotation
		ImGui::TextColored(white, "Rotation: ");
		ImGui::SameLine();

		float3 rotation = this->rotation_euler;
		if (ImGui::DragFloat3("##rot", rotation_euler.ptr(), 1.0f, -360.0f, 360.0f))
		{
			App->input->InfiniteHorizontal();
			SetRotation(rotation_euler);
		}

		//Scale
		ImGui::TextColored(white, "Scale:  ");
		ImGui::SameLine();

		float3 scale = this->scale;
		if (ImGui::DragFloat3("##scale", scale.ptr()))
		{
			App->input->InfiniteHorizontal();
			SetScale(scale);
		}

		if (ImGui::RadioButton("World", App->editor->gizmo_mode == ImGuizmo::MODE::WORLD))
		{
			App->editor->gizmo_mode = ImGuizmo::MODE::WORLD;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Local", App->editor->gizmo_mode == ImGuizmo::MODE::LOCAL))
		{
			App->editor->gizmo_mode = ImGuizmo::MODE::LOCAL;
		}

		//Local Matrix
		if (debug == true)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
			ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[0][0], transform_matrix.v[0][1], transform_matrix.v[0][2], transform_matrix.v[0][3]);
			ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[1][0], transform_matrix.v[1][1], transform_matrix.v[1][2], transform_matrix.v[1][3]);
			ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[2][0], transform_matrix.v[2][1], transform_matrix.v[2][2], transform_matrix.v[2][3]);
			ImGui::Text("%0.2f %0.2f %0.2f %0.2f", transform_matrix.v[3][0], transform_matrix.v[3][1], transform_matrix.v[3][2], transform_matrix.v[3][3]);
			ImGui::PopStyleColor();
		}
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

void ComponentTransform::Set(math::float4x4 matrix)
{
	transform_modified = true;

	float3 pos, scal;
	Quat rot;
	matrix.Decompose(pos, rot, scal);

	SetPosition(pos);
	SetRotation(rot);
	SetScale(scal);
}

void ComponentTransform::SetGlobal(float4x4 global)
{
	GameObject* parent = game_object->GetParent();
	if (parent != nullptr)
	{
		float4x4 new_local = ((ComponentTransform*)parent->GetComponent(C_TRANSFORM))->GetGlobalMatrix().Inverted() * global;
		float3 translate, scale;
		Quat rotation;
		new_local.Decompose(translate, rotation, scale);
		SetPosition(translate);
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

math::float4x4 ComponentTransform::GetLocalTransformMatrix() const
{
	return transform_matrix;
}

math::float4x4 ComponentTransform::GetTransformMatrix()const
{
	return final_transform_matrix.Transposed();
}

math::float4x4 ComponentTransform::GetGlobalMatrix()const
{
	return final_transform_matrix;
}

math::float3 ComponentTransform::GetForward() const
{
	return final_transform_matrix.WorldZ();
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

void ComponentTransform::Reset()
{
	SetPosition(float3::zero);
	SetRotation(Quat::identity);
	SetScale(float3::one);
}

void ComponentTransform::Remove()
{
	LOG("[WARNING] Component Transform from gameobject %s can't be removed.", GetGameObject()->name);
	App->editor->DisplayWarning(WarningType::W_WARNING, "Component Transform from gameobject %s can't be removed.", GetGameObject()->name);
}

void ComponentTransform::SaveAsPrefab(Data & file) const
{
	Data data;

	data.AppendFloat3("position", position.ptr());
	data.AppendFloat3("rotation", rotation_euler.ptr()); //Euler rotation

	file.AppendArrayValue(data);
}

void ComponentTransform::UpdateMatrix()
{
	if (transform_modified)
	{
		transform_matrix = transform_matrix.FromTRS(position, rotation, scale);
		CalculateFinalTransform();
		transform_modified = false;
	}
}

void ComponentTransform::CalculateFinalTransform()
{
	BROFILER_CATEGORY("ComponentTransform::CalculateFinalTransform", Profiler::Color::Chocolate)
	GameObject* game_object = GetGameObject();
	if (game_object)
	{
		if (game_object->GetParent())
		{
			final_transform_matrix = game_object->GetParent()->transform->final_transform_matrix * transform_matrix;

			std::vector<GameObject*>::const_iterator go_childs = game_object->GetChilds()->begin();
			for (go_childs; go_childs != game_object->GetChilds()->end(); ++go_childs)
				(*go_childs)->transform->CalculateFinalTransform();
		}
		else
		{
			final_transform_matrix = transform_matrix;
		}	

		game_object->TransformModified();
	}
	else
	{	
		LOG("[WARNING] Component created but not attached to any gameobject");
		App->editor->DisplayWarning(WarningType::W_WARNING, "Component created but not attached to any gameobject");
	}
}

void ComponentTransform::Rotate(const math::Quat & quaternion)
{
	rotation = quaternion * rotation;
	transform_modified = true;
}
