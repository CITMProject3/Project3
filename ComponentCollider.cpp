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
	SetShape(S_CUBE);
}

ComponentCollider::~ComponentCollider()
{
	
}

void ComponentCollider::Update()
{

	if (App->IsGameRunning() == false)
	{
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		body->SetTransform(trs->GetTransformMatrix().ptr());
		body->SetPos(trs->GetPosition().x, trs->GetPosition().y, trs->GetPosition().z);		
	}
	else
	{
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		float4x4 tmp = body->GetTransform().Transposed();
		tmp.Translate(tmp.Float3x3Part() * offset_pos);
		trs->Set(tmp);
	}
	transformModified = false;

	if (primitive)
	{		
		float3 translate;
		Quat rotation;
		float3 scale;
		body->GetTransform().Transposed().Decompose(translate, rotation, scale);
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

		ImGui::Separator();

		if (ImGui::Button("Remove ###cam_rem"))
		{
			Remove();
		}
	}
}

void ComponentCollider::OnTransformModified()
{
	transformModified = true;
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
	if (shape != new_shape)
	{
		if (shape != S_NONE && body != nullptr)
		{
			App->physics->RemoveBody(body);
		}
		if (primitive != nullptr)
		{
			delete primitive;
		}
		shape = new_shape;
		ComponentMesh* msh = (ComponentMesh*)game_object->GetComponent(C_MESH);
		if (msh)
		{
			ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
			offset_pos = msh->GetBoundingBox().CenterPoint() - trs->GetPosition();
		}

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
			body = App->physics->AddBody(*((Cube_P*)primitive));
			break;
		case S_SPHERE:
			if (msh)
			{
				primitive = new Sphere_P(msh->GetLocalAABB().Diagonal().Length()/2.0f);
			}
			else
			{
				primitive = new Sphere_P(1);
			}
			body = App->physics->AddBody(*((Sphere_P*)primitive));
			break;
		}

		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		body->SetTransform(trs->GetTransformMatrix().ptr());
		body->SetPos(trs->GetPosition().x, trs->GetPosition().y, trs->GetPosition().z);
	}
}
