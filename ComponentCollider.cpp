#include "Application.h"
#include "ComponentCollider.h"
#include "imgui\imgui.h"
#include "DebugDraw.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"

#include "ModulePhysics3D.h"
#include "PhysBody3D.h"

#include "ModuleInput.h"

ComponentCollider::ComponentCollider(GameObject* game_object) : Component(C_COLLIDER, game_object), shape(S_NONE)
{
}

ComponentCollider::~ComponentCollider()
{
	
}

void ComponentCollider::Update()
{
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);

	if (App->IsGameRunning() == false)
	{
		exists = false;
		if (primitive != nullptr)
		{
			//Setting the primitive pos
			float3 translate;
			Quat rotation;
			float3 scale;
			trs->GetGlobalMatrix().Decompose(translate, rotation, scale);
			translate += offset_pos;
			primitive->SetPos(translate.x, translate.y, translate.z);
			primitive->SetRotation(rotation.Inverted());

			primitive->Render();
		}
		if (shape == S_CONVEX)
		{
			//for(body->)
		}
	}
	else
	{
		if (exists == false)
		{
			LoadShape();
			exists = true;
		}
		if (primitive != nullptr)
		{
			//Setting the primitive pos
			float3 translate;
			Quat rotation;
			float3 scale;
			body->GetTransform().Transposed().Decompose(translate, rotation, scale);
			primitive->SetPos(translate.x, translate.y, translate.z);
			primitive->SetRotation(rotation.Inverted());
			primitive->Render();
			float3 real_offset = rotation.Transform(offset_pos);
			trs->Set(float4x4::FromTRS(translate - real_offset, rotation, trs->GetScale()));
		}
		if (shape == S_CONVEX)
		{
			trs->Set(body->GetTransform().Transposed());
		}
	}
	return;
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
		if (App->IsGameRunning() == false)
		{
			if (ImGui::BeginMenu("Shape: "))
			{
				if (ImGui::MenuItem("Cube", NULL))
				{
					SetShape(S_CUBE);
				}
				if (ImGui::MenuItem("Sphere", NULL))
				{
					SetShape(S_SPHERE);
				}
				if (ImGui::MenuItem("Convex mesh", NULL))
				{
					SetShape(S_CONVEX);
				}
				ImGui::EndMenu();
			}

			ImGui::SameLine();
			if (shape == S_CUBE) { ImGui::Text("Cube"); }
			if (shape == S_SPHERE) { ImGui::Text("Sphere"); }
			if (shape == S_CONVEX) { ImGui::Text("Convex mesh"); }

			ImGui::NewLine();
			ImGui::Checkbox("Static object: ", &Static);
			ImGui::DragFloat("Mass: ", &mass, 1.0f, 1.0f, 10000.0f);
			if (shape == S_CUBE || shape == S_SPHERE)
			{
				ImGui::DragFloat3("Collider offset: ", offset_pos.ptr(), 0.1f, -1000.0f, 1000.0f);
				if (shape == S_CUBE)
				{
				//	ImGui::DragFloat3("Scale offset: ", primitive->.ptr(), 0.1f, -1000.0f, 1000.0f);
				}
			}
		}
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
	data.AppendBool("static", Static);
	data.AppendFloat("mass", mass);
	data.AppendFloat3("offset_pos", offset_pos.ptr());

	file.AppendArrayValue(data);
}

void ComponentCollider::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	shape = (Collider_Shapes) conf.GetInt("shape");
	Static = conf.GetBool("static");
	mass = conf.GetFloat("mass");
	offset_pos = conf.GetFloat3("offset_pos");
	SetShape(shape);
}

void ComponentCollider::SetShape(Collider_Shapes new_shape)
{
	if (primitive != nullptr)
	{
		delete primitive;
		primitive = nullptr;
	}
	
	ComponentMesh* msh = (ComponentMesh*)game_object->GetComponent(C_MESH);
	if (msh && shape != new_shape)
	{
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		offset_pos = msh->GetBoundingBox().CenterPoint() - trs->GetPosition();
	}
	shape = new_shape;
	switch (new_shape)
	{
	case S_CUBE:
		if (msh)
		{
			primitive = new Cube_P(msh->GetLocalAABB().Size().x, msh->GetLocalAABB().Size().y, msh->GetLocalAABB().Size().z);
		}
		else
		{
			primitive = new Cube_P(1, 1, 1);
		}
		break;
	case S_SPHERE:
		if (msh)
		{
			primitive = new Sphere_P(msh->GetLocalAABB().Diagonal().Length() / 2.0f);
		}
		else
		{
			primitive = new Sphere_P(1);
		}
		break;
	case S_CONVEX:
		if (msh == false)
		{
			shape == S_NONE;
		}
		break;
	}
}

void ComponentCollider::LoadShape()
{
	if (shape != S_NONE)
	{
		float _mass = mass;
		if (Static)
		{
			_mass = 0.0f;
		}

		switch (shape)
		{
		case S_CUBE:
			body = App->physics->AddBody(*((Cube_P*)primitive), _mass);
			body->SetTransform(primitive->transform.ptr());
			break;
		case S_SPHERE:
			body = App->physics->AddBody(*((Sphere_P*)primitive), _mass);
			body->SetTransform(primitive->transform.ptr());
			break;
		case S_CONVEX:
			ComponentMesh* msh = (ComponentMesh*)game_object->GetComponent(C_MESH);
			ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
			body = App->physics->AddBody(*msh, _mass);
			body->SetTransform(trs->GetGlobalMatrix().ptr());
		}
		
	}
}
