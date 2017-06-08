#include "Application.h"
#include "ComponentCollider.h"
#include "imgui\imgui.h"
#include "DebugDraw.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ModuleRenderer3D.h"

#include "ModulePhysics3D.h"
#include "PhysBody3D.h"

#include "imgui\imgui.h"

#include "ModuleInput.h"
#include "glut\glut.h"

#include "Bullet\include\BulletCollision\CollisionShapes\btShapeHull.h"
#include "Brofiler\include\Brofiler.h"


ComponentCollider::ComponentCollider(GameObject* game_object) : Component(C_COLLIDER, game_object), shape(S_NONE)
{
	SetShape(S_CUBE);
	trigger_type = TriggerType::T_ON_TRIGGER;
	GetTriggerTypeName();
}

ComponentCollider::~ComponentCollider()
{ }

void ComponentCollider::Update()
{
	BROFILER_CATEGORY("ComponentCollider::Update", Profiler::Color::Green)

	if (App->IsGameRunning() == false || Static == true)
	{
		if (primitive != nullptr)
		{
			//Setting the primitive pos
			float3 translate;
			Quat rotation;
			float3 scale;
			game_object->transform->GetGlobalMatrix().Decompose(translate, rotation, scale);
			float3 real_offset = rotation.Transform(offset_pos);
			translate -= real_offset;
			primitive->SetPos(translate.x, translate.y, translate.z);
			primitive->SetRotation(rotation.Inverted());
			if (Static && App->IsGameRunning() == true && body != nullptr)
			{
				body->SetTransform(primitive->transform.ptr());
			}

			if (App->StartInGame() == false && App->physics->renderColliders)
			{
				primitive->Render();
			}
		}
	}
	else
	{
		if (primitive != nullptr)
		{
			//Setting the primitive pos
			float3 translate;
			Quat rotation;
			float3 scale;
			body->GetTransform().Transposed().Decompose(translate, rotation, scale);
			primitive->SetPos(translate.x, translate.y, translate.z);
			primitive->SetRotation(rotation.Inverted());
			if (App->StartInGame() == false && App->physics->renderColliders)
			{
				primitive->Render();
			}
			float3 real_offset = rotation.Transform(offset_pos);
			game_object->transform->Set(float4x4::FromTRS(translate + real_offset, rotation, game_object->transform->GetScale()));
		}
	}

	//Rendering Convex shapes
	if (App->IsGameRunning() && body != nullptr)
	{
		if (shape == S_CONVEX)
		{
			if (convexShape != nullptr)
			{
				int nEdges = convexShape->getNumEdges();
				for (int n = 0; n < nEdges; n++)
				{
					glPushMatrix();
					glMultMatrixf(body->GetTransform().ptr());
					btVector3 a, b;
					convexShape->getEdge(n, a, b);
					App->renderer3D->DrawLine(float3(a.x(), a.y(), a.z()), float3(b.x(), b.y(), b.z()));
					glPopMatrix();
				}
			}
		}
	}

	return;
}

void ComponentCollider::OnPlay()
{
	if (primitive != nullptr)
	{
		//Setting the primitive pos
		float3 translate;
		Quat rotation;
		float3 scale;
		game_object->transform->GetGlobalMatrix().Decompose(translate, rotation, scale);
		float3 real_offset = rotation.Transform(offset_pos);
		translate += real_offset;
		primitive->SetPos(translate.x, translate.y, translate.z);
		primitive->SetRotation(rotation.Inverted());
	}
	LoadShape();
}

void ComponentCollider::OnStop()
{
	convexShape = nullptr;
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
		if (debug || App->IsGameRunning() == false)
		{
			ImGui::NewLine();
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
			if (shape != S_CONVEX)
			{
				ImGui::Checkbox("Static object: ", &Static);
			}
			ImGui::DragFloat("Mass: ", &mass, 1.0f, 1.0f, 10000.0f);
			if (shape == S_CUBE || shape == S_SPHERE)
			{
				ImGui::DragFloat3("Collider offset: ", offset_pos.ptr(), 0.1f, -1000.0f, 1000.0f);
				if (shape == S_CUBE)
				{					
					ImGui::DragFloat3("Size: ", ((Cube_P*)primitive)->size.ptr(), 0.1f, -1000.0f, 1000.0f);
				}
				else if (shape == S_SPHERE)
				{
					ImGui::DragFloat("Radius", &((Sphere_P*)primitive)->radius, 0.1f, 0.1f, 1000.0f);					
				}
			}
		}
		ImGui::Separator();

		ImGui::Checkbox("Transparent", &isTransparent);
		ImGui::SameLine();

		ImGui::Checkbox("Trigger", &is_trigger);

		if (is_trigger)
		{
			if (ImGui::BeginMenu(trigger_type_name.c_str()))
			{
				if (ImGui::MenuItem("On Trigger"))
					trigger_type = TriggerType::T_ON_TRIGGER;
				if (ImGui::MenuItem("On Enter"))
					trigger_type = TriggerType::T_ON_ENTER;
				if (ImGui::MenuItem("On Exit"))
					trigger_type = TriggerType::T_ON_EXIT;
				GetTriggerTypeName();
				ImGui::EndMenu();
			}
		}
		
		

		ImGui::Separator();
		if (ImGui::Button("Remove ###col_rem"))
		{
			Remove();
		}
	}
}

void ComponentCollider::GetTriggerTypeName()
{
	switch (trigger_type)
	{
	case(TriggerType::T_ON_TRIGGER): trigger_type_name = "On Trigger"; break;
	case(TriggerType::T_ON_ENTER): trigger_type_name = "On Enter"; break;
	case(TriggerType::T_ON_EXIT): trigger_type_name = "On Exit"; break;
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

	data.AppendInt("CheckpointN", n);

	data.AppendInt("shape", shape);
	data.AppendBool("static", Static);
	data.AppendFloat("mass", mass);
	data.AppendFloat3("offset_pos", offset_pos.ptr());

	data.AppendBool("is_trigger", is_trigger);
	data.AppendInt("trigger_type", trigger_type);
	data.AppendBool("is_transparent", isTransparent);

	switch (shape)
	{
	case S_CUBE:
		data.AppendFloat3("size", ((Cube_P*)primitive)->size.ptr());
		break;
	case S_SPHERE:
		data.AppendFloat("radius", ((Sphere_P*)primitive)->radius);
		break;
	}

	file.AppendArrayValue(data);
}

void ComponentCollider::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	shape = (Collider_Shapes) conf.GetInt("shape");
	Static = conf.GetBool("static");
	mass = conf.GetFloat("mass");

	SetShape(shape);

	offset_pos = conf.GetFloat3("offset_pos");
	is_trigger = conf.GetBool("is_trigger");
	trigger_type = (TriggerType)conf.GetInt("trigger_type");
	GetTriggerTypeName();
	isTransparent = conf.GetBool("is_transparent");

	switch (shape)
	{
	case S_CUBE:
		((Cube_P*)primitive)->size = conf.GetFloat3("size");
		break;
	case S_SPHERE:
		((Sphere_P*)primitive)->radius = conf.GetFloat("radius");
		break;
	}
	
	n = conf.GetInt("CheckpointN");
}

void ComponentCollider::SetShape(Collider_Shapes new_shape)
{
	if (primitive != nullptr)
	{
		delete primitive;
		primitive = nullptr;
	}
	convexShape = nullptr;

	ComponentMesh* msh = (ComponentMesh*)game_object->GetComponent(C_MESH);
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
			shape = S_NONE;
		}
		else
		{
			Static = true;
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
		{
			body = App->physics->AddBody(*((Cube_P*)primitive), this, _mass, isTransparent, is_trigger, trigger_type);
			body->SetTransform(primitive->transform.ptr());
			break;
		}
		case S_SPHERE:
		{
			body = App->physics->AddBody(*((Sphere_P*)primitive), this, _mass, isTransparent, is_trigger, trigger_type);
			body->SetTransform(primitive->transform.ptr());
			break;
		}
		case S_CONVEX:
		{
			ComponentMesh* msh = (ComponentMesh*)game_object->GetComponent(C_MESH);
			body = App->physics->AddBody(*msh, this, _mass, isTransparent, is_trigger, trigger_type, &convexShape);
			break;
		}
		}
		
	}
}
