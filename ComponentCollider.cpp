#include "Application.h"
#include "ComponentCollider.h"
#include "imgui\imgui.h"
#include "DebugDraw.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "ModulePhysics3D.h"
#include "PhysBody3D.h"

ComponentCollider::ComponentCollider(GameObject* game_object) : Component(C_COLLIDER, game_object), shape(S_NONE)
{
	SetShape(S_CUBE);
}

ComponentCollider::~ComponentCollider()
{
	
}

void ComponentCollider::Update()
{
	float tmp[16];
	body->GetTransform(tmp);
	float4x4 physBodyTrs;
	physBodyTrs.Set(tmp);
	
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	trs->Set(physBodyTrs);
	
}

void ComponentCollider::OnInspector(bool debug)
{
	string str = (string("Collider") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##collider");
		}

		if (ImGui::BeginPopup("delete##collider"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginMenu("Shape: "))
		{
			if (ImGui::MenuItem("Cube", NULL))
			{
				shape = S_CUBE;
			}
			if (ImGui::MenuItem("Plane", NULL))
			{
				shape = S_PLANE;
			}
			if (ImGui::MenuItem("Sphere", NULL))
			{
				shape = S_SPHERE;
			}
			ImGui::EndMenu();
		}

		ImGui::SameLine();
		if(shape == S_CUBE) { ImGui::Text("Cube"); }
		if (shape == S_PLANE) { ImGui::Text("Plane"); }
		if (shape == S_SPHERE) { ImGui::Text("Sphere"); }

		ImGui::Separator();

		if (ImGui::Button("Remove ###cam_rem"))
		{
			Remove();
		}
	}
}

void ComponentCollider::OnTransformModified()
{

}

void ComponentCollider::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	data.AppendInt("shape", shape);

	file.AppendArrayValue(data);
}

void ComponentCollider::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	shape = (Collider_Shapes) conf.GetInt("shape");
}

void ComponentCollider::SetShape(Collider_Shapes new_shape)
{
	shape = new_shape;

	body = App->physics->AddBody(Cube_P(1, 1, 1));

	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	body->SetTransform(trs->GetTransformMatrix().ptr());	
}
