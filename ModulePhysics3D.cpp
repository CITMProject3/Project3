#include "ModulePhysics3D.h"

#include "Glew\include\glew.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ModuleLighting.h"
#include "ModuleCamera3D.h"
#include "ModuleGOManager.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentCar.h"
#include "ComponentCollider.h"
#include "ComponentScript.h"

#include "PhysBody3D.h"
#include "PhysVehicle3D.h"
#include "Primitive.h"

#include "Assets.h"
#include "RaycastHit.h"
#include "Time.h"

#include "Devil/include/il.h"
#include "Devil/include/ilut.h"

#include "Bullet\include\BulletCollision\CollisionShapes\btShapeHull.h"
#include "Bullet\include\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"

#include "ResourceFileTexture.h"

#include "SDL\include\SDL_scancode.h"

#include "Brofiler/include/Brofiler.h"

#define DUMMY_NUMBER 161803398


#define READ_TEX_VAL(n, u) ((u >> n*8) & 0xff)

#define SET_TEX_VAL(a1, a2, a3, a4, a5, a6, a7, a8)	((a8 << 54) | (a7 << 48) | (a6 << 40) | (a5 << 32) | (a4 << 24) | (a3 << 16) | (a2 << 8) | a1)

uint64_t set_tex_val(unsigned char val, unsigned int n, uint64_t storage)
{
	*(((char*)&storage) + n) = val;
	return storage;
}

#ifdef _DEBUG
	#pragma comment (lib, "Bullet/libx86/BulletDynamics_debug.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision_debug.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath_debug.lib")
#else
	#pragma comment (lib, "Bullet/libx86/BulletDynamics.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath.lib")
#endif

ModulePhysics3D::ModulePhysics3D(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	debug = true;

	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	debug_draw = new DebugDrawer(); // DEBUG DISABLED
}

// Destructor
ModulePhysics3D::~ModulePhysics3D()
{
	//delete debug_draw;
	delete solver;
	delete broad_phase;
	delete dispatcher;
	delete collision_conf;
}


// Render not available yet----------------------------------
bool ModulePhysics3D::Init(Data& config)
{
	LOG("Creating 3D Physics simulation");
	bool ret = true;

	return ret;
}

bool ModulePhysics3D::Start()
{
	LOG("Creating Physics environment");

	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	//world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);
	vehicle_raycaster = new btDefaultVehicleRaycaster(world);
	CreateGround();
	GetShaderLocations();
	return true;
}

update_status ModulePhysics3D::PreUpdate()
{
	BROFILER_CATEGORY("ModulePhysics3D::PreUpdate", Profiler::Color::YellowGreen)

	UpdateTriggerList();

	float dt = time->DeltaTime();
	if (App->IsGameRunning())
	{
		world->stepSimulation(dt, 15);

		int numManifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; i++)
		{
			btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
			btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
			btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

			int numContacts = contactManifold->getNumContacts();
			if (numContacts > 0)
			{
				PhysBody3D* pbodyA = (PhysBody3D*)obA->getUserPointer();
				PhysBody3D* pbodyB = (PhysBody3D*)obB->getUserPointer();

				if (pbodyA && pbodyB)
					OnCollision(pbodyA, pbodyB);
			}
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::Update()
{
	BROFILER_CATEGORY("ModulePhysics3D::Update", Profiler::Color::LimeGreen)

		if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
			debug = !debug;

	//TESTING

	unsigned int a = 0x000000FF;
	unsigned int b = 0x0000FF00;
	unsigned int c = 0x00FF0000;
	unsigned int d = 0xFF000000;

	uint64_t storage = 0;	
	storage = set_tex_val(10, 0, storage);
	storage = set_tex_val(20, 1, storage);
	storage = set_tex_val(30, 2, storage);
	storage = set_tex_val(40, 3, storage);
	storage = set_tex_val(50, 4, storage);
	storage = set_tex_val(60, 5, storage);
	storage = set_tex_val(70, 6, storage);
	storage = set_tex_val(80, 7, storage);

	unsigned char a1 = READ_TEX_VAL(0, storage);
	unsigned char a2 = READ_TEX_VAL(1, storage);
	unsigned char a3 = READ_TEX_VAL(2, storage);
	unsigned char a4 = READ_TEX_VAL(3, storage);
	unsigned char a5 = READ_TEX_VAL(4, storage);
	unsigned char a6 = READ_TEX_VAL(5, storage);
	unsigned char a7 = READ_TEX_VAL(6, storage);
	unsigned char a8 = READ_TEX_VAL(7, storage);

	int b8 = (storage >> 56) & 0xff;
	int b7 = (storage >> 48) & 0xff;
	int b6 = (storage >> 40) & 0xff;
	int b5 = (storage >> 32) & 0xff;
	int b4 = (storage >> 24) & 0xff;  // high-order (leftmost) byte: bits 24-31
	int b3 = (storage >> 16) & 0xff;  // next byte, counting from left: bits 16-23
	int b2 = (storage >> 8) & 0xff;  // next byte, bits 8-15
	int b1 = storage & 0xff;

	unsigned char res[12];
	for (size_t i = 0; i < sizeof(storage); ++i) {
		// Convert to unsigned char* because a char is 1 byte in size.
		// That is guaranteed by the standard.
		// Note that is it NOT required to be 8 bits in size.
		res[i] = *((unsigned char *)&storage + i);
	}

	int end = 4;

	/////////


	if (debug == true)
	{
		world->debugDrawWorld();
	}

	if (App->IsGameRunning() == false)
	{
		if (sculpted)
		{
			sculptTimer += time->RealDeltaTime();
			if (sculptTimer > 6.0f)
			{
				sculptTimer = 0.0f;
				sculpted = false;
				for (int n = 0; n < terrainW * terrainH; n++)
				{
					terrainData[n] = vertices[n].y;
					realTerrainData[n] = terrainData[n] / terrainMaxHeight;
				}
				ReinterpretNormals();
				ReinterpretHeightmapImg();
			}
		}

		if (currentTerrainTool != none_tool && terrainData != nullptr)
		{
			CAP(brushSize, 0, 1000);

			Ray ray = App->camera->GetEditorCamera()->CastCameraRay(float2(App->input->GetMouseX(), App->input->GetMouseY()));
			RaycastHit hit;
			bool ray_contact = RayCast(ray, hit);
			if (ray_contact)
			{
				int x = ceil(hit.point.x) - 1;
				int y = ceil(hit.point.z);
				x += terrainW / 2;
				y += terrainH / 2;
				int p = (y * terrainW + x) * 2;
#pragma region paintBrush
				glLineWidth(4.0f);
				for (int _y = y - brushSize - 1; _y < y + brushSize; _y++)
				{
					int x1 = x + brushSize + 1;
					int x2 = x - brushSize;
					if (_y > 1 && _y < terrainH - 1)
					{
						if (x1 > 1 && x1 < terrainW - 1)
						{
							App->renderer3D->DrawLine(vertices[_y * terrainW + x1], vertices[(_y + 1) * terrainW + x1], float4(0,1,1,1));
						}
						if (x2 > 1 && x2 < terrainW - 1)
						{
							App->renderer3D->DrawLine(vertices[_y * terrainW + x2], vertices[(_y + 1) * terrainW + x2], float4(0, 1, 1, 1));
						}
					}
				}

				for (int _x = x - brushSize; _x < x + brushSize + 1; _x++)
				{
					int y1 = y + brushSize;
					int y2 = y - brushSize - 1;
					if (_x > 1 && _x < terrainW - 1)
					{
						if (y1 > 0 && y1 < terrainH)
						{
							App->renderer3D->DrawLine(vertices[y1 * terrainW + _x], vertices[y1  * terrainW + _x + 1], float4(0, 1, 1, 1));
						}
						if (y2 > 0 && y2 < terrainH)
						{
							App->renderer3D->DrawLine(vertices[y2 * terrainW + _x], vertices[y2 * terrainW + _x + 1], float4(0, 1, 1, 1));
						}
					}
				}
				glLineWidth(1.0f);
#pragma endregion

#pragma region sculptMode
				if (currentTerrainTool == sculpt_tool)
				{
					if (App->input->GetMouseButton(1) == KEY_REPEAT || App->input->GetMouseButton(1) == KEY_DOWN)
					{
						if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
						{
							Sculpt(x, y, true);
						}
						else
						{
							Sculpt(x, y);
						}
					}
				}
#pragma endregion

#pragma region paintMode
				if (currentTerrainTool == paint_tool)
				{
					if (App->input->GetMouseButton(1) == KEY_REPEAT || App->input->GetMouseButton(1) == KEY_DOWN)
					{
						paintTexture = CAP(paintTexture, 0, 10);
						brushStrength = CAP(brushStrength, 0.1f, 99.0f);

						float maxVal = 0;

						float val;
						float newVal;
						uint textureN;
						for (int _y = -brushSize; _y <= brushSize; _y++)
						{
							for (int _x = -brushSize; _x <= brushSize; _x++)
							{
								if (_x + x > 0 && _y + y > 0 && _x + x < terrainW && _y + y < terrainH)
								{
									val = textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2];
									textureN = GetTextureN(val);

									if (hardBrush)
									{
										newVal = 1.0f;
									}
									else
									{
										//We're dividing it by two, because we want the radius to be half the length of the brush Size, not a whole size
										float dist = _x * _x + _y * _y;										
										float a = brushSize * brushSize * 0.75f;										
										dist = dist / a;
										maxVal = max(dist, maxVal);
										newVal = (1 - dist);
										newVal /= 10.0f;
										newVal = CAP(newVal, 0, 0.1f);
									}
									//Here, newVal should be a value between 0 and 1.
									// Here is where we transform it to the proper value
									newVal = CAP(newVal, 0, brushStrength / 1000.0f);
									

									if (paintTexture == textureN)
									{		
										val = max(GetTextureStrength(val), newVal);
										val = float(textureN / 10.0f) + val;
										if (val - float(textureN/10.0f) > 0.09f)
										{
											val = (textureN / 10.0f) + 0.09;
											textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2 + 1] = (textureN / 10.0f) + 0.05f;
										}
										textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2] = val;
									}
									else
									{
										
										if (hardBrush)
										{
											if (newVal < 0.09f)
											{
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2 + 1] = (textureN / 10.0f) + 0.05f;
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2] = (paintTexture / 10.0f) + newVal;
											}
											else
											{
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2 + 1] = (paintTexture / 10.0f) + 0.05f;
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2] = (paintTexture / 10.0f) + 0.09;
											}
										}
										else
										{
											if (paintTexture == GetTextureN(textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2 + 1]))
											{
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2 + 1] = textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2];
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2] = paintTexture / 10.0f + 0.1 - GetTextureStrength(textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2]);
											}
											else
											{
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2 + 1] = (textureN / 10.0f) + 0.05f;
												textureMap[((terrainH - (_y + y)) * terrainW + _x + x) * 2] = (paintTexture / 10.0f) + 0.0001f;
											}
										}
									}
								}
							}
						}
						ReinterpretTextureMap();
					}
				}
#pragma endregion

#pragma region PlaceGO Mode
				if (currentTerrainTool == goPlacement_tool)
				{
					std::vector<int> layersToCheck(size_t(60));
					for (int n = 0; n < 60; n++)
					{
						layersToCheck[n] = n;
					}
					Ray GOs_ray = App->camera->GetEditorCamera()->CastCameraRay(float2(App->input->GetMouseX(), App->input->GetMouseY()));
					RaycastHit GOs_hit = App->go_manager->Raycast(ray,layersToCheck);
					if (GOs_hit.object != nullptr && GOs_hit.distance < hit.distance)
					{
						hit = GOs_hit;
					}

					if (App->input->GetMouseButton(1) == KEY_DOWN && GO_toPaint_libPath.length() > 4)
					{
						Quat rot = Quat::identity;
						if (hit.normal.AngleBetween(float3(0, 1, 0)) > 5 * DEGTORAD)
						{
							rot = Quat::RotateFromTo(float3(0, 1, 0), hit.normal);
						}
						PlaceGO(hit.point, rot);
						last_placed_go->SetLayerChilds(60);
					}
					if (App->input->GetMouseButton(1) == KEY_REPEAT && GO_toPaint_libPath.length() > 4 && last_placed_go != nullptr)
					{
						ComponentTransform* trs = (ComponentTransform*)last_placed_go->GetComponent(ComponentType::C_TRANSFORM);
						if (trs != nullptr)
						{
							Quat rot = Quat::identity;

							if (hit.normal.AngleBetween(float3(0, 1, 0)) > 5 * DEGTORAD)
							{
								rot = Quat::RotateFromTo(float3(0, 1, 0), hit.normal);
							}
							trs->SetPosition(hit.point);
							trs->SetRotation(rot);
						}
					}
					if (App->input->GetMouseButton(1) == KEY_UP && GO_toPaint_libPath.length() > 4 && last_placed_go != nullptr)
					{
						last_placed_go->SetLayerChilds(0);
					}
				}
#pragma endregion
			}
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::PostUpdate()
{
	BROFILER_CATEGORY("ModulePhysics3D::PostUpdate", Profiler::Color::Green)
		return UPDATE_CONTINUE;
}

bool ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");

	CleanWorld();

	DeleteHeightmap();
	for (uint n = 0; n < GetNTextures(); n++)
	{
		DeleteTexture(n);
	}

	for (uint n = 0; n < GetNTextures(); n++)
	{
		DeleteTexture(n);
	}

	if (terrainOriginalUvBuffer != 0)
	{
		glDeleteBuffers(1, (GLuint*)&terrainOriginalUvBuffer);
		terrainOriginalUvBuffer = 0;
	}

	delete vehicle_raycaster;
	delete world;

	return true;
}

void ModulePhysics3D::GetShaderLocations()
{
	shader_id = App->resource_manager->GetDefaultTerrainShaderId();

	model_location = glGetUniformLocation(shader_id, "model");
	projection_location = glGetUniformLocation(shader_id, "projection");
	view_location = glGetUniformLocation(shader_id, "view");
	n_texs_location = glGetUniformLocation(shader_id, "_nTextures");
	tex_distributor_location = glGetUniformLocation(shader_id, "_TextureDistributor");
	texture_location_0 = glGetUniformLocation(shader_id, "_Texture_0");
	texture_location_1 = glGetUniformLocation(shader_id, "_Texture_1");
	texture_location_2 = glGetUniformLocation(shader_id, "_Texture_2");
	texture_location_3 = glGetUniformLocation(shader_id, "_Texture_3");
	texture_location_4 = glGetUniformLocation(shader_id, "_Texture_4");
	texture_location_5 = glGetUniformLocation(shader_id, "_Texture_5");
	texture_location_6 = glGetUniformLocation(shader_id, "_Texture_6");
	texture_location_7 = glGetUniformLocation(shader_id, "_Texture_7");
	texture_location_8 = glGetUniformLocation(shader_id, "_Texture_8");
	texture_location_9 = glGetUniformLocation(shader_id, "_Texture_9");
	has_tex_location = glGetUniformLocation(shader_id, "_HasTexture");
	texture_location = glGetUniformLocation(shader_id, "_Texture");
	colorLoc = glGetUniformLocation(shader_id, "material_color");
	ambient_intensity_location = glGetUniformLocation(shader_id, "_AmbientIntensity");
	ambient_color_location = glGetUniformLocation(shader_id, "_AmbientColor");
	has_directional_location = glGetUniformLocation(shader_id, "_HasDirectional");
	directional_intensity_location = glGetUniformLocation(shader_id, "_DirectionalIntensity");
	directional_color_location = glGetUniformLocation(shader_id, "_DirectionalColor");
	directional_direction_location = glGetUniformLocation(shader_id, "_DirectionalDirection");
}

void ModulePhysics3D::UpdateTriggerList()
{
	ComponentScript *script = nullptr;

	for (std::list<TriggerState*>::iterator it = triggers.begin(); it != triggers.end();)
	{
		PhysBody3D *body = (*it)->body;
		switch (body->trigger_type)
		{
			// Trigger with T_ON_ENTER flag will be erased from list when 
			case(TriggerType::T_ON_ENTER):
			{
				if (!(*it)->last_frame_check)
				{
					delete (*it);
					it = triggers.erase(it);
				}
				else
					++it;

				break;
			}
			case(TriggerType::T_ON_EXIT):
			{
				if (!(*it)->last_frame_check)
				{
					script = body->IsCar() ?
						(ComponentScript*)body->GetCar()->GetGameObject()->GetComponent(C_SCRIPT) :
						(ComponentScript*)body->GetCollider()->GetGameObject()->GetComponent(C_SCRIPT);

					// Call script
					if (script) script->OnCollision(body);

					delete (*it);
					it = triggers.erase(it);
				}
				else
					++it;

				break;
			}
		}
	}

	// Resetting values for all triggers
	for (std::list<TriggerState*>::iterator it = triggers.begin(); it != triggers.end(); ++it)
	{
		(*it)->last_frame_check = false;
	}		
}

bool ModulePhysics3D::CheckTriggerType(PhysBody3D *body)
{
	bool launch_trigger = false;

	switch (body->trigger_type)
	{
	case(TriggerType::T_ON_TRIGGER):
	{
		launch_trigger = true;
		break;
	}
	case(TriggerType::T_ON_ENTER):
	{
		bool on_list = false;
		for (std::list<TriggerState*>::iterator it = triggers.begin(); it != triggers.end(); ++it)
		{
			if (body == (*it)->body)
			{
				on_list = true;
				(*it)->last_frame_check = true;
			}			
		}

		// First time entering, so trigger will be called for ON_ENTER triggers
		if (!on_list)
		{
			triggers.push_back(new TriggerState(body));
			launch_trigger = true;
		}

		break;
	}
	case(TriggerType::T_ON_EXIT):
	{
		bool on_list = false;
		for (std::list<TriggerState*>::iterator it = triggers.begin(); it != triggers.end(); ++it)
		{
			if (body == (*it)->body)
			{
				on_list = true;
				(*it)->last_frame_check = true;
			}
		}

		// First time entering, so trigger will added and check on UpdateTriggersList()
		if (!on_list)
			triggers.push_back(new TriggerState(body));

		break;
	}
	}

	return launch_trigger;
}

void ModulePhysics3D::OnCollision(PhysBody3D *bodyA, PhysBody3D *bodyB)
{
	ComponentScript* script = nullptr;

	// BodyA
	if (bodyA->IsTrigger() && CheckTriggerType(bodyA))
	{
		script = bodyA->IsCar() ?
			(ComponentScript*)bodyA->GetCar()->GetGameObject()->GetComponent(C_SCRIPT) :
			(ComponentScript*)bodyA->GetCollider()->GetGameObject()->GetComponent(C_SCRIPT);

		// Call script
		if(script) script->OnCollision(bodyB);
	}

	// BodyB
	if (bodyB->IsTrigger() && CheckTriggerType(bodyB))
	{
		script = bodyB->IsCar() ?
			(ComponentScript*)bodyB->GetCar()->GetGameObject()->GetComponent(C_SCRIPT) :
			(ComponentScript*)bodyB->GetCollider()->GetGameObject()->GetComponent(C_SCRIPT);

		// Call script
		if (script) script->OnCollision(bodyA);
	}
}

void ModulePhysics3D::OnPlay()
{
	AddTerrain();
}

void ModulePhysics3D::OnStop()
{
	CleanWorld();
}

void ModulePhysics3D::CleanWorld()
{
	terrain = nullptr;

	// Remove from the world all collision bodies
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	for (list<btTypedConstraint*>::iterator item = constraints.begin(); item != constraints.end(); item++)
	{
		world->removeConstraint(*item);
		delete *item;
	}

	constraints.clear();

	for (list<btDefaultMotionState*>::iterator item = motions.begin(); item != motions.end(); item++)
		delete *item;

	motions.clear();

	for (list<btCollisionShape*>::iterator item = shapes.begin(); item != shapes.end(); item++)
		delete *item;

	shapes.clear();

	for (list<PhysBody3D*>::iterator item = bodies.begin(); item != bodies.end(); item++)
	{
		delete *item;
	}		

	bodies.clear();

	for (list<PhysVehicle3D*>::iterator item = vehicles.begin(); item != vehicles.end(); item++)
	{
		world->removeVehicle((*item)->vehicle);
		delete *item;
	}

	vehicles.clear();

	for (list<TriggerState*>::iterator item = triggers.begin(); item != triggers.end(); item++)
	{
		delete *item;
	}

	triggers.clear();
	
	CreateGround();
	world->clearForces();
}


// ---------------------------------------------------------
void ModulePhysics3D::CreateGround()
{
	// Big plane as ground
	btCollisionShape* colShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

	btDefaultMotionState* myMotionState = new btDefaultMotionState();
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape);

	btRigidBody* body = new btRigidBody(rbInfo);
	world->addRigidBody(body);
}

bool ModulePhysics3D::RayCast(Ray raycast, RaycastHit & hit_OUT)
{
	BROFILER_CATEGORY("ModulePhysics3D::Terrain_Raycast", Profiler::Color::HoneyDew);

	RaycastHit hit_info;
	bool ret = false;

	std::map<float, chunk> firstPass;
	float dNear = 0;
	float dFar = 0;

	for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
	{
		for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
		{			
			if (raycast.Intersects(it_x->second.GetAABB(), dNear, dFar))
			{
				firstPass.insert(std::pair<float, chunk>(dNear,it_x->second));
			}
		}
	}

	uint u1, u2, u3;
	float distance;
	vec hit_point;
	Triangle triangle;

	for (std::map<float, chunk>::iterator it = firstPass.begin(); it != firstPass.end(); it++)
	{
		for (uint n = 0; n < it->second.GetNIndices(); n += 3)
		{
			u1 = it->second.indices[n];
			u2 = it->second.indices[n + 1];
			u3 = it->second.indices[n + 2];
			triangle = Triangle(vertices[u1], vertices[u2], vertices[u3]);

			if (raycast.Intersects(triangle, &distance, &hit_point))
			{
				ret = true;
				if (hit_OUT.distance > distance || hit_OUT.distance == 0)
				{
					hit_OUT.distance = distance;
					hit_OUT.point = hit_point;
					hit_OUT.normal = triangle.NormalCCW();
				}
			}
		}
		if (ret == true)
		{
			hit_OUT.object = nullptr;
			hit_OUT.normal.Normalize();
			return true;
		}
	}	
	return false;
}

bool ModulePhysics3D::GenerateHeightmap(string resLibPath)
{	
	BROFILER_CATEGORY("ModulePhysics3D::Generate_Heightmap", Profiler::Color::HoneyDew);
	bool ret = false;
	//Loading Heightmap Image
	if (resLibPath != "" && resLibPath != " ")
	{
		ResourceFile* res = App->resource_manager->LoadResource(resLibPath, ResourceFileType::RES_TEXTURE);
		if (res != nullptr && res->GetType() == ResourceFileType::RES_TEXTURE)
		{
			DeleteHeightmap();
			

			//If the file exists and is loaded succesfully, we need to reload it manually.
			//The Resource won't hold the pixel data
			//Don't worry, we'll be deleting soon
			char* buffer = nullptr;
			unsigned int size = App->file_system->Load(res->GetFile(), &buffer);
			if (size > 0)
			{
				ResourceFileTexture* heightMapImg = (ResourceFileTexture*)res;

				ILuint id;
				ilGenImages(1, &id);
				ilBindImage(id);
				if (ilLoadL(IL_DDS, (const void*)buffer, size))
				{
					int width = ilGetInteger(IL_IMAGE_WIDTH);
					int height = ilGetInteger(IL_IMAGE_HEIGHT);
					BYTE* tmp = new BYTE[width * height * 3];
					//Copying all RGB data of each pixel into a uchar (BYTE) array. We need to transform it into float numbers
					realTerrainData = new float[width * height];
					terrainData = new float[width * height];
					ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, tmp);

					terrainW = heightMapImg->GetWidth();
					terrainH = heightMapImg->GetHeight();

					float* R = new float[width * height];
					float* G = new float[width * height];
					float* B = new float[width * height];

					for (int n = 0; n < width * height; n++)
					{
						R[n] = tmp[n * 3];
						G[n] = tmp[n * 3 + 1];
						B[n] = tmp[n * 3 + 2];
					}
					InterpretHeightmapRGB(R, G, B);

					delete[] R;
					delete[] G;
					delete[] B;

					//Deleting the second loaded image
					ilBindImage(0);
					ilDeleteImages(1, &id);
					ret = true;

					delete[] tmp;
					GenerateTerrainMesh();

					if (heightMapImg != nullptr)
					{
						heightMapImg->Unload();
					}
				}
			}
			RELEASE_ARRAY(buffer);
		}
	}
	return ret;
}

void ModulePhysics3D::DeleteHeightmap()
{
	if (terrainData != nullptr)
	{
		delete[] terrainData;
		terrainData = nullptr;
	}
	if (realTerrainData != nullptr)
	{
		delete[] realTerrainData;
		realTerrainData = nullptr;
	}
	DeleteTerrainMesh();
	terrainW = terrainH = 0;
}

bool ModulePhysics3D::SaveTextureMap(const char * path)
{	
	if (terrainData)
	{
		// -------- Collecting all chunks and its data into a single array -------------------------------------------------
		struct pos_chunk
		{
			pos_chunk(chunk a, int b, int c) : Chunk(a), posX(b), posZ(c) {}
			chunk Chunk;
			int posX;
			int posZ;
		};

		uint w = terrainW;
		uint h = terrainH;

		std::vector<pos_chunk> toSave;

		for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
		{
			for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
			{
				toSave.push_back(pos_chunk(it_x->second, it_x->first, it_z->first));
			}
		}

		//Getting ready to store chunks. Uint stores the amount of indices
		uint size_totalChunkSize = 0; //byte size of all chunks together
		std::vector<std::pair<uint, char*>> chunkData;
		for (std::vector<pos_chunk>::iterator it_chunk = toSave.begin(); it_chunk != toSave.end(); it_chunk++)
		{
			std::pair<uint, char*> data;
			uint totalSize = 0; //Total size of this chunk
			//Coordinates
			totalSize += sizeof(int) * 2;
			//AABB
			totalSize += sizeof(float3) * 2;
			//Indices
			totalSize += sizeof(uint) * it_chunk->Chunk.GetNIndices();

			data.first = it_chunk->Chunk.GetNIndices();

			//Size of the chunk data + the uint that stores the length
			size_totalChunkSize += totalSize + sizeof(uint);

			//Generating the buffer to save this chunk data
			data.second = new char[totalSize];
			char* buf_it = data.second;

			

			//Coordinate X, Z
			uint bytes = sizeof(int);
			memcpy(buf_it, &it_chunk->posX, bytes);
			buf_it += bytes;
			memcpy(buf_it, &it_chunk->posZ, bytes);
			buf_it += bytes;

			//AABB
			bytes = sizeof(float3);
			memcpy(buf_it, it_chunk->Chunk.GetAABB().minPoint.ptr(), bytes);
			buf_it += bytes;
			memcpy(buf_it, it_chunk->Chunk.GetAABB().maxPoint.ptr(), bytes);
			buf_it += bytes;

			bytes = sizeof(uint) * data.first;
			memcpy(buf_it, it_chunk->Chunk.indices, bytes);

			chunkData.push_back(data);
		}

		//Terrain size (heightmap width, heightmap height, texture scaling, n chunks, Dummy number, version, max height)
		uint size_generalData = sizeof(uint) * (3 + 2) + sizeof(float) * 2;
		//Number of vertices
		uint size_vertices = sizeof(float3) * w * h;
		//Normals
		uint size_normals = sizeof(float3) * w * h;
		//Texture map
		uint size_textureMap = sizeof(float) * w * h * 2 + sizeof(uint);
		//Number of textures
		uint texturesSize = sizeof(uint) * 1;
		for (int n = 0; n < GetNTextures(); n++)
		{
			//Name Length && Path length
			texturesSize += sizeof(uint) * 2;
			texturesSize += sizeof(char) * (GetTexturePath(n).length() + 1);
			texturesSize += sizeof(char) * (GetTextureName(n).length() + 1);
		}


		//We're not saving UVs, we can regenerate them fastly

		long unsigned int size_total = size_generalData + size_totalChunkSize + size_vertices + size_normals + size_textureMap + texturesSize;
		char* buf = new char[size_total];
		char* it = buf;

		uint bytes = sizeof(uint);
		uint dummy = DUMMY_NUMBER;
		memcpy(it, &dummy, bytes);
		it += bytes;

		dummy = TERRAIN_VERSION;
		memcpy(it, &dummy, bytes);
		it += bytes;

		//Terrain width		
		memcpy(it, &w, bytes);
		it += bytes;
		//Terrain height
		memcpy(it, &h, bytes);
		it += bytes;

		//Max height
		bytes = sizeof(float);
		memcpy(it, &terrainMaxHeight, bytes);
		it += bytes;

		//Texture scaling
		memcpy(it, &textureScaling, bytes);
		it += bytes;

		//Vertices
		bytes = sizeof(float3) * w * h;
		memcpy(it, vertices, bytes);
		it += bytes;

		//Normals
		bytes = sizeof(float3) * w * h;
		memcpy(it, normals, bytes);
		it += bytes;

		//Texture map
		bytes = sizeof(uint);
		memcpy(it, &textureMapScale, bytes);
		it += bytes;

		bytes = sizeof(float) * w * h * textureMapScale * 2;
		memcpy(it, textureMap, bytes);
		it += bytes;

		//Number of chunks
		bytes = sizeof(uint);
		uint nChunks = chunkData.size();
		memcpy(it, &nChunks, bytes);
		it += bytes;

		for (std::vector<std::pair<uint, char*>>::iterator it_data = chunkData.begin(); it_data != chunkData.end(); it_data++)
		{
			//Chunks
			bytes = sizeof(uint);
			memcpy(it, &it_data->first, bytes);
			it += bytes;

			bytes = it_data->first * sizeof(uint) + sizeof(int) * 2 + sizeof(float3) * 2;
			memcpy(it, it_data->second, bytes);
			it += bytes;

			delete[] it_data->second;
		}		

		uint nTextures = GetNTextures();
		bytes = sizeof(uint);
		memcpy(it, &nTextures, bytes);
		it += bytes;

		for (int n = 0; n < nTextures; n++)
		{
			uint pathLen = GetTexturePath(n).length() + 1;
			uint nameLen = GetTextureName(n).length() + 1;
			bytes = sizeof(uint);
			memcpy(it, &pathLen, bytes);
			it += bytes;
			memcpy(it, &nameLen, bytes);
			it += bytes;

			bytes = sizeof(char) * pathLen;
			memcpy(it, GetTexturePath(n).data(), bytes);
			it += bytes;
			
			bytes = sizeof(char) * nameLen;
			memcpy(it, GetTextureName(n).data(), bytes);
			it += bytes;
		}

		bool ret = App->file_system->Save(path, buf, size_total);

		RELEASE_ARRAY(buf);

		return ret;
		
	}
	return false;
}

bool ModulePhysics3D::LoadTextureMap(const char * path)
{
	if (path != nullptr && path != "" && path != " ")
	{
		if (App->file_system->Exists(path) == false) { return false; }

		BROFILER_CATEGORY("ModulePhysics3D::LoadHeightmap", Profiler::Color::HoneyDew);
		char* tmp = nullptr;
		uint size = App->file_system->Load(path, &tmp);
		if (size > 0)
		{
			char* it = tmp;

			uint dummy = 0;
			uint bytes = sizeof(uint);
			memcpy(&dummy, it, bytes);
			it += bytes;

			uint version = 0;
			//Since not all terrains have a version number at the start, the ones that have it all begin with this "Dummy number"
			//Any document that begins with a dummy number is followed by an uint that defines the terrain version
			//Any document that doesn't have it, is version 0 and must be read and loaded from the first byte
			if (dummy == DUMMY_NUMBER)
			{
				memcpy(&version, it, bytes);
				it += bytes;
			}
			else
			{
				version = 0;
				it = tmp;
			}
			
			//Terrain w
			memcpy(&terrainW, it, bytes);
			it += bytes;
			//Terrain H
			memcpy(&terrainH, it, bytes);
			it += bytes;

			//Terrain max height
			bytes = sizeof(float);
			memcpy(&terrainMaxHeight, it, bytes);
			it += bytes;

			if (version >= 2)
			{
				memcpy(&textureScaling, it, bytes);
				it += bytes;
			}

			RELEASE_ARRAY(vertices);
			RELEASE_ARRAY(terrainData);
			RELEASE_ARRAY(realTerrainData);

			vertices = new float3[terrainW * terrainH];
			terrainData = new float[terrainW * terrainH];
			realTerrainData = new float[terrainW * terrainH];

			bytes = sizeof(float3) * terrainW * terrainH;
			memcpy(vertices, it, bytes);
			it += bytes;

			for (int n = 0; n < terrainW * terrainH; n++)
			{
				terrainData[n] = vertices[n].y;
				realTerrainData[n] = terrainData[n] / terrainMaxHeight;
			}

			RELEASE_ARRAY(normals);
			bytes = sizeof(float3) * terrainW * terrainH;
			normals = new float3[terrainW * terrainH];
			memcpy(normals, it, bytes);
			it += bytes;

			bytes = sizeof(uint);
			memcpy(&textureMapScale, it, bytes);
			it += bytes;

			if (textureMapScale == 0)
			{
				textureMapScale = 1;
			}

			RELEASE_ARRAY(textureMap);
			textureMap = new float[terrainW * terrainH * textureMapScale * 2];
			if (version == 0)
			{
				float* tmp_textureMap = new float[terrainW * terrainH * textureMapScale];
				bytes = sizeof(float) * terrainW * terrainH * textureMapScale;
				memcpy(tmp_textureMap, it, bytes);
				it += bytes;

				for (int n = 0; n < terrainW * terrainH * textureMapScale; n++)
				{
					textureMap[n * 2] = tmp_textureMap[n];
					textureMap[n * 2 + 1] = textureMap[n * 2];
				}
				RELEASE_ARRAY(tmp_textureMap);

			}
			else if (version >= 1)
			{
				bytes = sizeof(float) * terrainW * terrainH * textureMapScale * 2;
				memcpy(textureMap, it, bytes);
				it += bytes;
			}

			uint nChunks;
			bytes = sizeof(uint);
			memcpy(&nChunks, it, bytes);
			it += bytes;

			chunks.clear();
			{
				BROFILER_CATEGORY("ModulePhysics3D::LoadHeightmap::GeneratingChunks", Profiler::Color::HoneyDew);
				for (int n = 0; n < nChunks; n++)
				{
					uint size;
					bytes = sizeof(uint);
					memcpy(&size, it, bytes);
					it += bytes;

					int coordX, coordZ;
					float3 minP, maxP;

					bytes = sizeof(int);
					memcpy(&coordX, it, bytes);
					it += bytes;
					memcpy(&coordZ, it, bytes);
					it += bytes;

					bytes = sizeof(float3);
					memcpy(&minP, it, bytes);
					it += bytes;
					memcpy(&maxP, it, bytes);
					it += bytes;


					std::map<int, std::map<int, chunk>>::iterator it_z = chunks.insert(std::pair<int, std::map<int, chunk>>(coordZ, std::map<int, chunk>())).first;
					std::map<int, chunk>::iterator it_x = it_z->second.insert(std::pair<int, chunk>(coordX, chunk())).first;

					it_x->second.indices = new uint[size];
					it_x->second.nIndices = size;
					it_x->second.SetAABB(minP, maxP);

					bytes = sizeof(uint) * size;
					memcpy(it_x->second.indices, it, bytes);
					it += bytes;

					it_x->second.GenBuffer();
				}
			}

			if (version >= 2)
			{
				uint nTextures;
				bytes = sizeof(uint);
				memcpy(&nTextures, it, bytes);
				it += bytes;

				for (uint n = 0; n < nTextures; n++)
				{
					uint pathLen;
					bytes = sizeof(uint);
					memcpy(&pathLen, it, bytes);
					it += bytes;
					uint nameLen;
					memcpy(&nameLen, it, bytes);
					it += bytes;

					char* texPath = new char[pathLen];
					char* texName = new char[nameLen];

					bytes = sizeof(char) * pathLen;
					memcpy(texPath, it, bytes);
					it += bytes;

					bytes = sizeof(char) * nameLen;
					memcpy(texName, it, bytes);
					it += bytes;

					LoadTexture(texPath, -1, texName);

					RELEASE_ARRAY(texPath);
					RELEASE_ARRAY(texName);
				}
			}

			RELEASE_ARRAY(tmp);

			{
				BROFILER_CATEGORY("ModulePhysics3D::LoadHeightmap::GenerateBuffers", Profiler::Color::HoneyDew);
				ReinterpretMesh();
				ReinterpretHeightmapImg();
				ReinterpretTextureMap();
				GenerateUVs();
			}
			return true;
		}
	}
	return false;
}

bool ModulePhysics3D::TerrainIsGenerated()
{
	return (terrainData != nullptr);
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Sphere_P& sphere, ComponentCollider* col, float mass, bool isTransparent, bool is_trigger, TriggerType t_type)
{
	btCollisionShape* colShape = new btSphereShape(sphere.radius);
	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(*sphere.transform.v);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body, col);

	pbody->SetTrigger(is_trigger, t_type);
	if(isTransparent) body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);


	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

PhysBody3D* ModulePhysics3D::AddBody(const Cube_P& cube, ComponentCollider* col, float mass, bool isTransparent, bool is_trigger, TriggerType t_type)
{
	btCollisionShape* colShape = new btBoxShape(btVector3(cube.size.x*0.5f, cube.size.y*0.5f, cube.size.z*0.5f));
	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(*cube.transform.v);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body, col);

	pbody->SetTrigger(is_trigger, t_type);
	if (isTransparent) body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

PhysBody3D* ModulePhysics3D::AddBody(const Cylinder_P& cylinder, ComponentCollider* col, float mass, bool isTransparent, bool is_trigger, TriggerType t_type)
{
	btCollisionShape* colShape = new btCylinderShapeX(btVector3(cylinder.height*0.5f, cylinder.radius, 0.0f));
	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(*cylinder.transform.v);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body, col);

	pbody->SetTrigger(is_trigger, t_type);
	if (isTransparent) body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

PhysBody3D* ModulePhysics3D::AddBody(const ComponentMesh& mesh, ComponentCollider* col, float mass, bool isTransparent, bool is_trigger, TriggerType t_type, btConvexHullShape** out_shape)
{
	btConvexHullShape* colShape = new btConvexHullShape();

	float3* vertices = (float3*)mesh.GetMesh()->vertices;
	uint nVertices = mesh.GetMesh()->num_vertices;

	for (uint n = 0; n < nVertices; n++)
	{
		colShape->addPoint(btVector3(vertices[n].x, vertices[n].y, vertices[n].z));
	}

	btShapeHull* hull = new btShapeHull(colShape);
	hull->buildHull(colShape->getMargin());
	btConvexHullShape* simplifiedColShape = new btConvexHullShape((btScalar*)hull->getVertexPointer(), hull->numVertices());

	shapes.push_back(simplifiedColShape);

	if (out_shape != nullptr)
	{
		*out_shape = simplifiedColShape;
	}

	btTransform startTransform;
	float4x4 go_trs = mesh.GetGameObject()->transform->GetGlobalMatrix().Transposed();
	startTransform.setFromOpenGLMatrix(go_trs.ptr());

	btVector3 localInertia(0, 0, 0);
	if (mass != 0.f)
		simplifiedColShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, simplifiedColShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body, col);

	pbody->SetTrigger(is_trigger, t_type);
	if (isTransparent) body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	delete hull;
	delete colShape;

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysVehicle3D* ModulePhysics3D::AddVehicle(const VehicleInfo& info, ComponentCar* col)
{
	btCompoundShape* comShape = new btCompoundShape();
	shapes.push_back(comShape);

	//Base
	btCollisionShape* colBase = new btBoxShape(btVector3(info.chassis_size.x*0.5f, info.chassis_size.y*0.5f, info.chassis_size.z*0.5f));
	shapes.push_back(colBase);
	
	btCollisionShape* colNose = new btBoxShape(btVector3(info.nose_size.x * 0.5f, info.nose_size.y* 0.5f, info.nose_size.z*0.5f));
	shapes.push_back(colNose);

	btTransform transBase;
	transBase.setIdentity();
	transBase.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y, info.chassis_offset.z));

	comShape->addChildShape(transBase, colBase);

	btTransform transNose;
	transNose.setIdentity();
	transNose.setOrigin(btVector3(info.nose_offset.x, info.nose_offset.y, info.nose_offset.z));

	comShape->addChildShape(transNose, colNose);

	btTransform startTransform;
	startTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);
	comShape->calculateLocalInertia(info.mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(info.mass, myMotionState, comShape, localInertia);
	
	btRigidBody* body = new btRigidBody(rbInfo);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	body->setActivationState(DISABLE_DEACTIVATION);
	body->setCcdMotionThreshold(1e-7);
	body->setCcdSweptSphereRadius(1.0);

	world->addRigidBody(body);

	btRaycastVehicle::btVehicleTuning tuning;
	tuning.m_frictionSlip = info.frictionSlip;
	tuning.m_maxSuspensionForce = info.maxSuspensionForce;
	tuning.m_maxSuspensionTravelCm = info.maxSuspensionTravelCm;
	tuning.m_suspensionCompression = info.suspensionCompression;
	tuning.m_suspensionDamping = info.suspensionDamping;
	tuning.m_suspensionStiffness = info.suspensionStiffness;

	btRaycastVehicle* vehicle = new btRaycastVehicle(tuning, body, vehicle_raycaster);

	vehicle->setCoordinateSystem(0, 1, 2);

	for(int i = 0; i < info.num_wheels; ++i)
	{
		btVector3 conn(info.wheels[i].connection.x, info.wheels[i].connection.y, info.wheels[i].connection.z);
		btVector3 dir(info.wheels[i].direction.x, info.wheels[i].direction.y, info.wheels[i].direction.z);
		btVector3 axis(info.wheels[i].axis.x, info.wheels[i].axis.y, info.wheels[i].axis.z);

		vehicle->addWheel(conn, dir, axis, info.wheels[i].suspensionRestLength, info.wheels[i].radius, tuning, info.wheels[i].front);
	}
	// ---------------------

	PhysVehicle3D* pvehicle = new PhysVehicle3D(body, vehicle, info, col);
	world->addVehicle(vehicle);
	vehicles.push_back(pvehicle);

	pvehicle->SetTrigger(true, TriggerType::T_ON_ENTER);
	pvehicle->SetCar(true);
	pvehicle->SetTransform(info.transform.Transposed().ptr());

	return pvehicle;
}

void ModulePhysics3D::Sculpt(int x, int y, bool inverse)
{
	if (x >= 0 && y >= 0 && x < terrainW && y < terrainH)
	{
		switch (sculptTool)
		{
		case sculpt_smooth:
		{
			if (brushSize > 0 && x >= 0 && y >= 0 && x < terrainW && y < terrainH)
			{
				float value = 0.0f;
				int n = 0;
				//Iterating all image pixels
				for (int _y = y - brushSize; _y <= y + brushSize; _y++)
				{
					if (_y >= 0 && _y < terrainH)
					{
						for (int _x = x - brushSize; _x <= x + brushSize; _x++)
						{
							if (_x >= 0 && _x < terrainW)
							{
								value = 0.0f;
								n = 0;
								//The 6 used in the for is a magic number. It's the kernel size

								//Iterating all nearby pixels and checking they actually exist in the image
								for (int _y2 = _y - 6; _y2 <= _y + 6; _y2++)
								{
									if (_y2 >= 0 && _y2 < terrainH)
									{
										for (int _x2 = _x - 6; _x2 <= _x + 6; _x2++)
										{
											if (_x2 >= 0 && _x2 < terrainW)
											{
												n++;
												value += terrainData[_y2 * terrainW + _x2];
											}
										}
									}
								}
								value /= n;

								if (math::Abs(vertices[_y * terrainW + _x].y - value) < brushStrength* time->RealDeltaTime())
								{
									vertices[_y * terrainW + _x].y = value;
								}
								else if (vertices[_y * terrainW + _x].y > value)
								{
									vertices[_y * terrainW + _x].y -= brushStrength* time->RealDeltaTime();
								}
								else
								{
									vertices[_y * terrainW + _x].y += brushStrength* time->RealDeltaTime();
								}
							}
						}
					}
				}
			}
			break;
		}
		case sculpt_raise:
		{
			for (int _y = y - brushSize; _y <= y + brushSize; _y++)
			{
				for (int _x = x - brushSize; _x <= x + brushSize; _x++)
				{
					if (_x >= 0 && _y >= 0 && _x < terrainW && _y < terrainH)
					{
						float displacement = brushStrength* time->RealDeltaTime();
						if (brushSize > 0)
						{
							float a = Max(math::Abs(x - _x), math::Abs(y - _y));
							displacement = brushStrength * time->RealDeltaTime() * (1.0f - (a / (float)brushSize));
						}
						if (inverse) { displacement *= -1; }
						vertices[_y * terrainW + _x].y += displacement;
						if (vertices[_y * terrainW + _x].y < 0)
						{
							vertices[_y * terrainW + _x].y = 0;
						}
					}
				}
			}
			break;
		}
		case sculpt_flatten:
		{
			float h = vertices[y * terrainW + x].y;
			for (int _y = y - brushSize - 1; _y <= y + brushSize; _y++)
			{
				for (int _x = x - brushSize; _x <= x + brushSize + 1; _x++)
				{
					if (_x >= 0 && _y >= 0 && _x < terrainW && _y < terrainH)
					{
						if (math::Abs(vertices[_y * terrainW + _x].y - h) < brushStrength* time->RealDeltaTime())
						{
							vertices[_y * terrainW + _x].y = h;
						}
						else if (vertices[_y * terrainW + _x].y > h)
						{
							vertices[_y * terrainW + _x].y -= brushStrength* time->RealDeltaTime();
						}
						else
						{
							vertices[_y * terrainW + _x].y += brushStrength* time->RealDeltaTime();
						}
					}
				}
			}
			break;
		}
		}
		ReinterpretVertices();
		RegenerateNormals(x - brushSize - 1, y - brushSize - 1, x + brushSize + 1, y + brushSize + 1);

		for (int _y = y - brushSize; _y <= y + brushSize; _y += min(CHUNK_H, brushSize))
		{
			for (int _x = x - brushSize; _x <= x + brushSize; _x += min(CHUNK_W, brushSize))
			{
				int chunkX = floor(_x / CHUNK_W);
				int chunkY = floor(_y / CHUNK_H);
				chunks[chunkY][chunkX].UpdateAABB();
				sculpted = true;
			}
		}
	}
}

void ModulePhysics3D::PlaceGO(float3 pos, Quat rot)
{
	GameObject* go = App->resource_manager->LoadFile(GO_toPaint_libPath, PREFAB);
	if (go == nullptr) { return; }
	ComponentTransform* trs = (ComponentTransform*)go->GetComponent(ComponentType::C_TRANSFORM);
	if (trs == nullptr) { return; }
	last_placed_go = go;
	trs->SetPosition(pos);
	trs->SetRotation(rot);
}


// ---------------------------------------------------------
void ModulePhysics3D::AddTerrain()
{
	if (terrainData)
	{
		terrain = new btHeightfieldTerrainShape(terrainW, terrainH, terrainData, 1.0f, -terrainMaxHeight - 1, terrainMaxHeight + 1, 1, PHY_ScalarType::PHY_FLOAT, false);
		shapes.push_back(terrain);

		btDefaultMotionState* myMotionState = new btDefaultMotionState();
		motions.push_back(myMotionState);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, terrain);

		btRigidBody* body = new btRigidBody(rbInfo);

		world->addRigidBody(body);
	}
}

void ModulePhysics3D::GenerateIndices()
{
	BROFILER_CATEGORY("ModulePhysics3D::Generate_Indices", Profiler::Color::HoneyDew);
	if (terrainData)
	{
		DeleteIndices();

		int w = terrainW;
		int h = terrainH;

		//Interior vertices all need 6 indices
		//limit vertices all need 3 indices
		//Except corner ones. Two need 1 and two need 2
		//numIndices = ((w - 2) * (h - 2)) * 6 + (w * 2 + h * 2) * 3 - 2 - 1 - 2 - 1;
		//indices = new uint[numIndices];

		{
			BROFILER_CATEGORY("ModulePhysics3D::Generate_Indices::AddingTris", Profiler::Color::HoneyDew);
			for (int z = 0; z < h - 1; z++)
			{
				for (int x = 0; x < w - 1; x++)
				{
					AddTriToChunk(((z + 1) * w + x), (z * w + x + 1), (z * w + x), x, z);
					AddTriToChunk((z * w + x + 1), ((z + 1) * w + x), ((z + 1) * w + x + 1), x, z);
				}
			}
		}

		{
			BROFILER_CATEGORY("ModulePhysics3D::Generate_Indices::UpdatingAABB_GenChunkBuffers", Profiler::Color::HoneyDew);
			for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
			{
				for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
				{
					it_x->second.UpdateAABB();
					it_x->second.GenBuffer();
				}
			}
		}
	}
}

void ModulePhysics3D::DeleteIndices()
{
	chunks.clear();
}


void ModulePhysics3D::UpdateChunksAABBs()
{
	for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
	{
		for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
		{
			it_x->second.UpdateAABB();
		}
	}
}

void ModulePhysics3D::AddTriToChunk(const uint& i1, const uint& i2, const uint& i3, int& x, int& z)
{
	int chunkX = floor(x / CHUNK_W);
	int chunkZ = floor(z / CHUNK_H);

	//Checking if the correspondant Z chunk exists. Creating it otherwise
	std::map<int, std::map<int, chunk>>::iterator it_z = chunks.find(chunkZ);
	if (it_z == chunks.end())
	{
		//Inserting a value returns a pair<iterator, bool>
		//The iterator is what we're interessted in
		//The bool shows if the value has been inserted or it already existed
		//It shouldn't be relevant, since we already checked if it existed
		it_z = chunks.insert(std::pair<int, std::map<int, chunk>>(chunkZ, std::map<int, chunk>())).first;
	}

	//Same process for the correspondant X coordinate
	std::map<int, chunk>::iterator it_x = it_z->second.find(chunkX);
	if (it_x == it_z->second.end())
	{
		it_x = it_z->second.insert(std::pair<int, chunk>(chunkX, chunk())).first;
	}
	
	it_x->second.AddIndex(i1);
	it_x->second.AddIndex(i2);
	it_x->second.AddIndex(i3);
}

std::vector<chunk> ModulePhysics3D::GetVisibleChunks(ComponentCamera* camera)
{
	BROFILER_CATEGORY("ModulePhysics3D::RenderTerrain::Getting visible chunks", Profiler::Color::HoneyDew);

	Frustum frust = camera->GetFrustum();
	vec corners[8];
	frust.GetCornerPoints(corners);
	AABB frust_box;
	frust_box.SetNegativeInfinity();
	frust_box.SetFrom(corners, 8);

	std::vector<chunk> firstPass;
	for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
	{
		for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
		{
			if (it_x->second.GetAABB().Intersects(frust_box))
			{
				firstPass.push_back(it_x->second);
			}
		}
	}


	std::vector<chunk> ret;
	for (std::vector<chunk>::iterator it = firstPass.begin(); it != firstPass.end(); it++)
	{
		if (it->GetAABB().Intersects(frust))
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

void ModulePhysics3D::RenderTerrain(ComponentCamera* camera)
{
	BROFILER_CATEGORY("ModulePhysics3D::RenderTerrain", Profiler::Color::HoneyDew);
	
	if (renderFilledTerrain)
	{
		RealRenderTerrain(camera, false);
	}
	if (renderWiredTerrain)
	{
		RealRenderTerrain(camera, true);
	}
}

void ModulePhysics3D::RealRenderTerrain(ComponentCamera * camera, bool wired)
{


	if (GetNChunksW() >= 0 && terrainData != nullptr)
	{
		if (wired)
		{
			glLineWidth(0.5f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		uint shader_id = App->resource_manager->GetDefaultTerrainShaderId();
		//Use shader
		glUseProgram(shader_id);

		//Set uniforms

		//Matrices
		glUniformMatrix4fv(model_location, 1, GL_FALSE, *(float4x4::identity).v);
		glUniformMatrix4fv(projection_location, 1, GL_FALSE, *camera->GetProjectionMatrix().v);
		glUniformMatrix4fv(view_location, 1, GL_FALSE, *camera->GetViewMatrix().v);

		glUniform1i(n_texs_location, textures.size());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureMapBufferID);
		glUniform1i(tex_distributor_location, 0);

		int count = 0;
		if (textures.size() > 0)
		{
			uint nTextures = textures.size();
			//TEXTURE 0
			if (0 < nTextures && wired == false)
			{
				glUniform1i(texture_location_0, 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textures[0].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 1			
			if (1 < nTextures && wired == false)
			{
				glUniform1i(texture_location_1, 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, textures[1].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 2
			if (2 < nTextures && wired == false)
			{
				glUniform1i(texture_location_2, 3);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, textures[2].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 3
			if (3 < nTextures && wired == false)
			{
				glUniform1i(texture_location_3, 4);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, textures[3].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 4
			if (4 < nTextures && wired == false)
			{
				glUniform1i(texture_location_4, 5);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, textures[4].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 5
			if (5 < nTextures && wired == false)
			{
				glUniform1i(texture_location_5, 6);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, textures[5].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 6
			if (6 < nTextures && wired == false)
			{
				glUniform1i(texture_location_6, 7);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, textures[6].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 7
			if (7 < nTextures && wired == false)
			{
				glUniform1i(texture_location_7, 8);
				glActiveTexture(GL_TEXTURE8);
				glBindTexture(GL_TEXTURE_2D, textures[7].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE8);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 8
			if (8 < nTextures && wired == false)
			{
				glUniform1i(texture_location_8, 9);
				glActiveTexture(GL_TEXTURE9);
				glBindTexture(GL_TEXTURE_2D, textures[8].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE9);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//TEXTURE 9
			if (9 < nTextures && wired == false)
			{
				glUniform1i(texture_location_9, 10);
				glActiveTexture(GL_TEXTURE10);
				glBindTexture(GL_TEXTURE_2D, textures[9].first->GetTexture());
			}
			else
			{
				glActiveTexture(GL_TEXTURE10);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

		}
		else
		{
			glUniform1i(has_tex_location, 1);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureMapBufferID);
			glUniform1i(texture_location, 0);
		}

		if (colorLoc != -1)
		{
			float4 color(1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4fv(colorLoc, 1, color.ptr());
		}


		//Lighting
		LightInfo light = App->lighting->GetLightInfo();
		//Ambient
		if (ambient_intensity_location != -1)
			glUniform1f(ambient_intensity_location, light.ambient_intensity);
		if (ambient_color_location != -1)
			glUniform3f(ambient_color_location, light.ambient_color.x, light.ambient_color.y, light.ambient_color.z);

		//Directional
		glUniform1i(has_directional_location, light.has_directional);

		if (light.has_directional)
		{
			if (directional_intensity_location != -1)
				glUniform1f(directional_intensity_location, light.directional_intensity);
			if (directional_color_location != -1)
				glUniform3f(directional_color_location, light.directional_color.x, light.directional_color.y, light.directional_color.z);
			if (directional_direction_location != -1)
				glUniform3f(directional_direction_location, light.directional_direction.x, light.directional_direction.y, light.directional_direction.z);
		}


		//Buffer vertices == 0
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, terrainVerticesBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		//Buffer uvs == 1
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, terrainUvBuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		//Buffer normals == 2
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, terrainNormalBuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, (GLvoid*)0);

		//Buffer terrainUVs == 4
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, terrainOriginalUvBuffer);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
		{
			for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
			{
				if (renderChunks)
				{
					it_x->second.Render();
					if (wired)
					{
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					}
				}
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it_x->second.GetBuffer());
				glDrawElements(GL_TRIANGLES, it_x->second.GetNIndices(), GL_UNSIGNED_INT, (void*)0);
			}
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void ModulePhysics3D::GenerateTerrainMesh()
{
	GenerateVertices();
	GenerateIndices();
	GenerateUVs();
	AutoGenerateTextureMap();
}

void ModulePhysics3D::DeleteNormals()
{
	RELEASE_ARRAY(normals);
	if (terrainNormalBuffer != 0)
	{
		glDeleteBuffers(1, (GLuint*)&terrainNormalBuffer);
		terrainNormalBuffer = 0;
	}
}

void ModulePhysics3D::GenerateUVs()
{
	if (terrainData)
	{
		int w = terrainW;
		int h = terrainH;

		DeleteUVs();

		float2* uvs = new float2[w*h];

		for (int z = 0; z < h; z++)
		{
			for (int x = 0; x < w; x++)
			{
				float uv_x = ((float)x / (float)w) / textureScaling;
				float uv_y = 1 - (((float)z / (float)h) / textureScaling);
				uvs[z * w + x] = float2(uv_x, uv_y);

			}
		}

		//Load UVs -----------------------------------------------------------------------------------------------------------------------
		glGenBuffers(1, (GLuint*)&(terrainUvBuffer));
		glBindBuffer(GL_ARRAY_BUFFER, terrainUvBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * w*h, uvs, GL_STATIC_DRAW);

		delete[] uvs;


		float2* originalUvs = new float2[w*h];

		for (int z = 0; z < h - 1; z++)
		{
			for (int x = 0; x < w - 1; x++)
			{
				originalUvs[z * w + x] = float2(((float)x / (float)w), (1 - ((float)z / (float)h)));
			}
		}

		//Load Original UVs -----------------------------------------------------------------------------------------------------------------------
		if (terrainOriginalUvBuffer == 0)
		{
			glGenBuffers(1, (GLuint*)&(terrainOriginalUvBuffer));
		}
		glBindBuffer(GL_ARRAY_BUFFER, terrainOriginalUvBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * w*h, originalUvs, GL_STATIC_DRAW);

		delete[] originalUvs;
	}
}

void ModulePhysics3D::DeleteUVs()
{
	if (terrainUvBuffer != 0)
	{
		glDeleteBuffers(1, (GLuint*)&terrainUvBuffer);
		terrainUvBuffer = 0;
	}
}

void ModulePhysics3D::DeleteTerrainMesh()
{
	DeleteVertices();
	DeleteNormals();
	DeleteUVs();
	DeleteIndices();
}

void ModulePhysics3D::GenerateVertices()
{
	if (terrainData)
	{
		int w = terrainW;
		int h = terrainH;
		uint numVertices = w * h;

		RELEASE_ARRAY(vertices);

		vertices = new float3[numVertices];

		for (int z = 0; z < h; z++)
		{
			for (int x = 0; x < w; x++)
			{
				vertices[z * w + x] = float3(x - w / 2, realTerrainData[z * w + x] * terrainMaxHeight, z - h / 2);
			}
		}
		GenerateNormals();

		ReinterpretMesh();
		ReinterpretHeightmapImg();
	}
}

void ModulePhysics3D::DeleteVertices()
{
	RELEASE_ARRAY(vertices);
	if (terrainVerticesBuffer != 0)
	{
		glDeleteBuffers(1, (GLuint*)&terrainVerticesBuffer);
		terrainVerticesBuffer = 0;
	}
}

void ModulePhysics3D::GenerateNormals()
{
	if (terrainData && vertices)
	{
		int w = terrainW;
		int h = terrainH;
		uint numVertices = w * h;

		RELEASE_ARRAY(normals);

		normals = new float3[numVertices];

		RegenerateNormals(0, 0, w, h);
	}
}

void ModulePhysics3D::InterpretHeightmapRGB(float * R, float * G, float * B)
{
	if (terrainData)
	{
		int w = terrainW;
		int h = terrainH;

		float* edgeH = new float[(w*h) * 3];
		float* edgeV = new float[(w*h) * 3];

		RELEASE_ARRAY(textureMap);
		textureMap = new float[w*h * 2];

		float* buf = new float[w*h];
		float maxVal = 0;

		//Setting the buffer content to the max value of RGB, to get a single matrix instead of three (R, G, B)
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				buf[y*w + x] = (max(max(R[y*w + x], G[y*w + x]), B[y*w + x]));
			}
		}

		float value = 0.0f;
		int n = 0;
		//Iterating all image pixels
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
#pragma region GaussianBlur
				value = 0.0f;
				n = 0;
				//Iterating all nearby pixels and checking they actually exist in the image
				for (int _y = y - terrainSmoothLevels; _y <= y + terrainSmoothLevels; _y++)
				{
					if (_y > 0 && _y < h)
					{
						for (int _x = x - terrainSmoothLevels; _x <= x + terrainSmoothLevels; _x++)
						{
							if (_x > 0 && _x < w)
							{
								n++;
								value += buf[_y * w + _x];
							}
						}
					}
				}
				value /= n;
				value /= 255;
				realTerrainData[y*w + x] = value;
				terrainData[y*w + x] = value * terrainMaxHeight;
#pragma endregion

#pragma region Edge detection
				int hk[3][3] = { {-3,0,3}, {-10,0,10}, {-3,0,3} };
				int vk[3][3] = { { -3,-10,-3 },{ 0,0,0 },{ 3,10,3 } };
				edgeH[y * w + x] = 0;
				edgeV[y * w + x] = 0;
				if (x - 1 >= 0 && x + 1 < w && y - 1 >= 0 && y + 1 < h)
				{
					for (int _y = -1; _y <= 1; _y++)
					{
						for (int _x = -1; _x <= 1; _x++)
						{
							edgeH[y * w + x] += buf[(y + _y) * w + x + _x] * hk[_y + 1][_x + 1];

							edgeV[y * w + x] += buf[(y + _y) * w + x + _x] * vk[_y + 1][_x + 1];
						}
					}
				}

				textureMap[((h - y - 1) * w + x) * 2] = math::Sqrt(edgeH[y * w + x] * edgeH[y * w + x] + edgeV[y * w + x] * edgeV[y * w + x]);
				if (textureMap[((h - y - 1) * w + x) * 2] > maxVal)
				{
					maxVal = textureMap[((h - y - 1) * w + x) * 2];
				}
#pragma endregion
			}
		}

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				textureMap[(y * w + x)*2] /= maxVal;
				textureMap[(y * w + x) * 2 + 1] = textureMap[(y * w + x) * 2];
			}
		}

		ReinterpretTextureMap();

		delete[] edgeH;
		delete[] edgeV;
		delete[] buf;
	}
}

uint ModulePhysics3D::GetTextureN(float textureValue)
{
	return floor(textureValue / 0.1f);
}

float ModulePhysics3D::GetTextureStrength(float textureValue)
{	
	return textureValue - floor(textureValue/0.1f)/10.0f;
}

void ModulePhysics3D::SetTerrainMaxHeight(float height)
{
	if (height > 0.1f)
	{
		if (terrainData && realTerrainData)
		{
			for (unsigned int n = 0; n < terrainW * terrainH; n++)
			{
				terrainData[n] = realTerrainData[n]  * height;
			}
		}
		terrainMaxHeight = height;
		GenerateVertices();
		UpdateChunksAABBs();
	}
}

void ModulePhysics3D::SetTextureScaling(float scale, bool doNotUse)
{
	if (scale > 0)
	{
		float tmp = 0.0f;
		if (doNotUse) { tmp = textureScaling; }
		textureScaling = scale;
		GenerateUVs();
		if (doNotUse) { textureScaling = tmp; }
	}
}

void ModulePhysics3D::LoadTexture(string resLibPath, int pos, string texName)
{
	//Loading Heightmap Image
	if (resLibPath != "" && resLibPath != " " && GetNTextures() <= 10)
	{
		if ((pos == -1) || (pos >= 0 && pos < GetNTextures()))
		{
			ResourceFile* res = App->resource_manager->LoadResource(resLibPath, ResourceFileType::RES_TEXTURE);
			if (res != nullptr && res->GetType() == ResourceFileType::RES_TEXTURE)
			{
				if (pos == -1)
				{
					textures.push_back(std::pair<ResourceFileTexture*, string>((ResourceFileTexture*)res, texName));
				}
				else
				{
					textures[pos].first->Unload();
					textures[pos].first = (ResourceFileTexture*)res;
					textures[pos].second = texName;
				}
				if (textures.size() == 1)
				{
					GenerateUVs();
				}
			}
		}
	}
}

void ModulePhysics3D::DeleteTexture(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		int x = 0;
		for (std::vector<std::pair<ResourceFileTexture*, string>>::iterator it = textures.begin(); it != textures.end(); it++)
		{
			if (x == n)
			{
				textures[n].first->Unload();
				textures.erase(it);
				break;
			}
			x++;
		}
	}

	if (textures.size() == 0)
	{
		SetTextureScaling(1.0f, true);
	}
}

int ModulePhysics3D::GetHeightmap()
{
	if (terrainData != nullptr)
	{
		return heightmap_bufferID;
	}
	return 0;
}

int ModulePhysics3D::GetTexture(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n].first->GetTexture();
	}
	return 0;
}

string ModulePhysics3D::GetTextureName(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n].second;
	}
	return string("");
}

uint ModulePhysics3D::GetTextureUUID(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n].first->GetUUID();
	}
	return 0;
}

string ModulePhysics3D::GetTexturePath(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n].first->GetFile();
	}
	return string("");
}

uint ModulePhysics3D::GetNTextures()
{
	return textures.size();
}

float2 ModulePhysics3D::GetHeightmapSize()
{
	return float2(terrainW, terrainH);
}

void ModulePhysics3D::AutoGenerateTextureMap()
{
	if (terrainData)
	{
		int w = terrainW;
		int h = terrainH;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				if (textureMap[(y * w + x) * 2] > 0.1)
				{
					textureMap[(y * w + x) * 2] = 0.19f;
				}
				else
				{
					textureMap[(y * w + x) * 2] = 0.09f;
				}
				textureMap[(y * w + x) * 2 + 1] = textureMap[(y * w + x) * 2];
			}
		}
		ReinterpretTextureMap();
	}
}

void ModulePhysics3D::ReinterpretTextureMap()
{
	if (terrainData)
	{
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		if (textureMapBufferID == 0)
		{
			glGenTextures(1, &textureMapBufferID);
		}
		glBindTexture(GL_TEXTURE_2D, textureMapBufferID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, terrainW, terrainH, 0, GL_RG, GL_FLOAT, textureMap);
	}
}

void ModulePhysics3D::ReinterpretHeightmapImg()
{
	if (terrainData)
	{
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		if (heightmap_bufferID == 0)
		{
			glGenTextures(1, &heightmap_bufferID);
		}
		glBindTexture(GL_TEXTURE_2D, heightmap_bufferID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		float3* tmp = new float3[terrainW * terrainH];
		memcpy(tmp, vertices, sizeof(float3) * terrainW * terrainH);
		float newTerrainMaxHeight = 0.0f;
		for (uint n = 0; n < terrainW * terrainH; n++)
		{
			newTerrainMaxHeight = Max(newTerrainMaxHeight, tmp[n].y);
		}
		for (uint n = 0; n < terrainW * terrainH; n++)
		{
			tmp[n].y = tmp[n].y / newTerrainMaxHeight;
			tmp[n].x = tmp[n].z = tmp[n].y;
		}

		terrainMaxHeight = newTerrainMaxHeight;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, terrainW, terrainH, 0, GL_RGB, GL_FLOAT, tmp);

		delete[] tmp;
	}
}

void ModulePhysics3D::ReinterpretMesh()
{
	ReinterpretVertices();
	ReinterpretNormals();
}

void ModulePhysics3D::ReinterpretVertices()
{
	//Load vertices buffer to VRAM
	if (terrainVerticesBuffer == 0)
	{
		glGenBuffers(1, (GLuint*)&(terrainVerticesBuffer));
	}
	glBindBuffer(GL_ARRAY_BUFFER, terrainVerticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * terrainW * terrainH, vertices, GL_STATIC_DRAW);
}

void ModulePhysics3D::ReinterpretNormals()
{
	//Load Normals -----------------------------------------------------------------------------------------------------------------------
	if (terrainNormalBuffer == 0)
	{
		glGenBuffers(1, (GLuint*)&(terrainNormalBuffer));
	}
	glBindBuffer(GL_ARRAY_BUFFER, terrainNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * terrainW * terrainH, normals, GL_STATIC_DRAW);
}

void ModulePhysics3D::RegenerateNormals(int x0, int y0, int x1, int y1)
{
	int w = terrainW;
	int h = terrainH;
	CAP(x0, 0, terrainW - 1);
	CAP(x1, 0, terrainW - 1);
	CAP(y0, 0, terrainH - 1);
	CAP(y1, 0, terrainH - 1);
	for (int z = y0; z < y1; z++)
	{
		for (int x = x0; x < x1; x++)
		{
			Triangle t;
			float3 norm = float3::zero;

			//Top left
			if (x - 1 > 0 && z - 1 > 0)
			{
				t.a = vertices[(z)* w + x];
				t.b = vertices[(z - 1)* w + x];
				t.c = vertices[(z)* w + x - 1];
				norm += t.NormalCCW();
			}
			//Top right
			if (x + 1 < terrainW && z - 1 > 0)
			{
				t.a = vertices[(z)* w + x];
				t.b = vertices[(z)* w + x + 1];
				t.c = vertices[(z - 1)* w + x];
				norm += t.NormalCCW();
			}
			//Bottom left
			if (x - 1 > 0 && z + 1 < terrainH)
			{
				t.a = vertices[(z)* w + x];
				t.b = vertices[(z)* w + x - 1];
				t.c = vertices[(z + 1)* w + x];
				norm += t.NormalCCW();
			}
			//Bottom right
			if (x + 1 < terrainW && z + 1 < terrainH)
			{
				t.a = vertices[(z)* w + x];
				t.b = vertices[(z + 1)* w + x];
				t.c = vertices[(z)* w + x + 1];
				norm += t.NormalCCW();
			}
			norm.Normalize();
			normals[z * w + x] = norm;
		}
	}

	ReinterpretNormals();
}


// ---------------------------------------------------------
void ModulePhysics3D::AddConstraintP2P(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec& anchorA, const vec& anchorB)
{
	btTypedConstraint* p2p = new btPoint2PointConstraint(
		*(bodyA.body), 
		*(bodyB.body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z), 
		btVector3(anchorB.x, anchorB.y, anchorB.z));
	world->addConstraint(p2p);
	constraints.push_back(p2p);
	p2p->setDbgDrawSize(2.0f);
}

void ModulePhysics3D::AddConstraintHinge(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec& anchorA, const vec& anchorB, const vec& axisA, const vec& axisB, bool disable_collision)
{
	btHingeConstraint* hinge = new btHingeConstraint(
		*(bodyA.body), 
		*(bodyB.body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z),
		btVector3(anchorB.x, anchorB.y, anchorB.z),
		btVector3(axisA.x, axisA.y, axisA.z), 
		btVector3(axisB.x, axisB.y, axisB.z));

	world->addConstraint(hinge, disable_collision);
	constraints.push_back(hinge);
	hinge->setDbgDrawSize(2.0f);
}


// =============================================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	line.origin.Set(from.getX(), from.getY(), from.getZ());
	line.destination.Set(to.getX(), to.getY(), to.getZ());
	line.color.Set(color.getX(), color.getY(), color.getZ());
	line.Render();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.transform.SetTranslatePart(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes) debugMode;
}

int	 DebugDrawer::getDebugMode() const
{
	return mode;
}

///// CHUNK ======================================

chunk::chunk(): indices()
{
	aabb.SetNegativeInfinity();
}

chunk::~chunk()
{
	if (indices_bufferID == 0)
	{
		glDeleteBuffers(1, (GLuint*)&indices_bufferID);
	}
}

void chunk::GenBuffer()
{
	if (nIndices > 0)
	{
		if (indices_bufferID == 0)
		{
			glGenBuffers(1, (GLuint*) &(indices_bufferID));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_bufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * nIndices, indices, GL_STATIC_DRAW);
	}
}

int chunk::GetBuffer()
{
	return indices_bufferID;
}

int chunk::GetNIndices()
{
	return nIndices;
}

const uint * chunk::GetIndices()
{
	return indices;
}

void chunk::AddIndex(const uint& i)
{
	if (nIndices + 1 >= avaliableSpace)
	{
		avaliableSpace = nIndices + 64;
		uint* tmp = new uint[avaliableSpace];
		memcpy(tmp, indices, sizeof(uint) * nIndices);
		RELEASE_ARRAY(indices);
		indices = tmp;
	}
	indices[nIndices] = i;
	nIndices++;
}

void chunk::UpdateAABB()
{
	aabb.SetNegativeInfinity();
	for (int n = 0; n < nIndices; n++)
	{
		aabb.Enclose(App->physics->vertices[indices[n]]);
	}
}

void chunk::CleanIndices()
{
	RELEASE_ARRAY(indices);
	nIndices = 0;
}

void chunk::Render()
{
	App->renderer3D->DrawAABB(aabb.minPoint, aabb.maxPoint, float4(0.674, 0.784, 0.886, 1.0f));
}

void chunk::SetAABB(float3 minPoint, float3 MaxPoint)
{
	aabb.minPoint = minPoint;
	aabb.maxPoint = MaxPoint;
}
