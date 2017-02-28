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
	}
	else
	{
		if (exists == false)
		{
			LoadShape();
			exists = true;
		}
		float4x4 tmp = body->GetTransform().Transposed();
		tmp.Translate(tmp.Float3x3Part() * offset_pos);
		trs->Set(tmp);
	}

	if (primitive)
	{		
		float3 translate;
		Quat rotation;
		float3 scale;
		trs->GetGlobalMatrix().Decompose(translate, rotation, scale);
		primitive->SetPos(translate.x, translate.y, translate.z);
		primitive->SetRotation(rotation.Inverted());
		primitive->Scale(scale.x, scale.y, scale.z);
		primitive->Render();
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
			ImGui::EndMenu();
		}

		ImGui::SameLine();
		if(shape == S_CUBE) { ImGui::Text("Cube"); }
		if (shape == S_SPHERE) { ImGui::Text("Sphere"); }

		ImGui::NewLine();
		ImGui::Checkbox("Static object: ", &Static);
		ImGui::DragFloat3("Collider offset: ", offset_pos.ptr());
		ImGui::DragFloat("Mass: ", &mass, 1.0f, 1.0f, 10000.0f);

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
	}
	shape = new_shape;
	ComponentMesh* msh = (ComponentMesh*)game_object->GetComponent(C_MESH);
	/*if (msh)
	{
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		offset_pos = msh->GetBoundingBox().CenterPoint() - trs->GetPosition();
	}*/

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
	}
}

void ComponentCollider::LoadShape()
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
		break;
	case S_SPHERE:
		body = App->physics->AddBody(*((Sphere_P*)primitive), _mass);
		break;
	}

	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	body->SetTransform(trs->GetTransformMatrix().ptr());
	//body->SetPos(trs->GetPosition().x, trs->GetPosition().y, trs->GetPosition().z);
}
