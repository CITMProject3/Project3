#include "ModulePhysics3D.h"

#include "Glew\include\glew.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ModuleLighting.h"
#include "ModuleCamera3D.h"

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

	if (debug == true)
	{
		world->debugDrawWorld();
	}

	if (paintMode && heightMapImg)
	{
		if (App->input->GetMouseButton(1) == KEY_REPEAT || App->input->GetMouseButton(1) == KEY_DOWN)
		{
			Ray ray = App->camera->GetEditorCamera()->CastCameraRay(float2(App->input->GetMouseX(), App->input->GetMouseY()));
			RaycastHit hit;
			if (RayCast(ray, hit))
			{
				CAP(paintTexture, 0, 10);

				int x = ceil(hit.point.x) - 1;
				int y = ceil(hit.point.z);
				x += heightMapImg->GetWidth() / 2;
				y += heightMapImg->GetHeight() / 2;
				for (int _y = -brushSize; _y <= brushSize; _y++)
				{
					for (int _x = -brushSize; _x <= brushSize; _x++)
					{
						if (_x + x > 0 && _y + y > 0 && _x + x < heightMapImg->GetWidth() && _y + y < heightMapImg->GetHeight())
						{
							textureMap[((heightMapImg->GetHeight() - (_y + y)) * heightMapImg->GetWidth() + _x + x)] = (paintTexture / 10.0f) + 0.05f;
						}
					}
				}
				ReinterpretTextureMap();
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

	if (heightMapImg)
	{
		heightMapImg->Unload();
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
		if(script) script->OnCollision(bodyA);
	}

	// BodyB
	if (bodyB->IsTrigger() && CheckTriggerType(bodyB))
	{
		script = bodyB->IsCar() ?
			(ComponentScript*)bodyB->GetCar()->GetGameObject()->GetComponent(C_SCRIPT) :
			(ComponentScript*)bodyB->GetCollider()->GetGameObject()->GetComponent(C_SCRIPT);

		// Call script
		if (script) script->OnCollision(bodyB);
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
	if (resLibPath != GetHeightmapPath() && resLibPath != "" && resLibPath != " ")
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
				heightMapImg = (ResourceFileTexture*)res;

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
				}
			}
			if (buffer != nullptr)
			{
				delete[] buffer;
			}
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
	if (heightMapImg != nullptr)
	{
		heightMapImg->Unload();
		heightMapImg = nullptr;
	}
	DeleteTerrainMesh();
}

bool ModulePhysics3D::SaveTextureMap(const char * path)
{	
	if (heightMapImg)
	{
		// -------- Collecting all chunks and its data into a single array -------------------------------------------------
		/*struct pos_chunk
		{
			pos_chunk(chunk a, uint b, uint c) : Chunk(a), posX(b), posZ(c) {}
			chunk Chunk;
			uint posX;
			uint posZ;
		};

		std::vector<pos_chunk> toSave;

		for (std::map<int, std::map<int, chunk>>::iterator it_z = chunks.begin(); it_z != chunks.end(); it_z++)
		{
			for (std::map<int, chunk>::iterator it_x = it_z->second.begin(); it_x != it_z->second.end(); it_x++)
			{
				toSave.push_back(pos_chunk(it_x->second, it_x->first, it_z->first));
			}
		}

		//Getting ready to store chunks. Uint stores the length of chunkData
		uint totalChunkSize = 0;
		std::vector<std::pair<uint, char*>> chunkData;
		for (std::vector<pos_chunk>::iterator it_chunk = toSave.begin(); it_chunk != toSave.end(); it_chunk++)
		{
			std::pair<uint, char*> data;
			data.first = 0;
			//Coordinates
			data.first += sizeof(uint) * 2;
			//AABB
			data.first += sizeof(float) * 6;
			//Indices
			data.first += sizeof(uint) * it_chunk->Chunk.indices.size();

			//Size of the chunk data + the uint that stores the length
			totalChunkSize += data.first + sizeof(uint);

			//Generating the buffer to save this chunk data
			data.second = new char[data.first];
			char* buf_it = data.second;

			//Coordinate X, Z
			uint bytes = sizeof(uint);
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

			bytes = sizeof(uint) * it_chunk->Chunk.indices.size();
			memcpy(buf_it, it_chunk->Chunk.indices.data(), bytes);
		}

		uint nVertices = sizeof(float) * heightMapImg->GetWidth() * heightMapImg->GetHeight();
		uint textureMapSize = sizeof(float) * heightMapImg->GetWidth() * heightMapImg->GetHeight();



		uint nIndices = 0;

		char* chunks;


		char* buf = new char[sizeof(uint) * 2 + heightMapSize + textureMapSize];
		char* it = buf;

		uint bytes = sizeof(uint);
		memcpy(it, &heightMapSize, bytes);
		it += bytes;

		memcpy(it, &textureMapSize, bytes);
		it += bytes;

		memcpy(it, vertices)*/

		return App->file_system->Save(path, textureMap, sizeof(float) * heightMapImg->GetWidth() * heightMapImg->GetHeight());
	}
	return false;
}

void ModulePhysics3D::LoadTextureMap(const char * path)
{
	if (heightMapImg)
	{
		char* tmp = nullptr;		
		uint size = App->file_system->Load(path, &tmp);		
		if (size > 0)
		{
			if (textureMap != nullptr)
			{
				delete[] textureMap;
			}
			textureMap = (float*)tmp;
		}
		ReinterpretTextureMap();
	}
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


// ---------------------------------------------------------
void ModulePhysics3D::AddTerrain()
{
	if (heightMapImg != nullptr)
	{
		terrain = new btHeightfieldTerrainShape(heightMapImg->GetWidth(), heightMapImg->GetHeight(), terrainData, 1.0f, -300, 300, 1, PHY_ScalarType::PHY_FLOAT, false);
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
	if (heightMapImg)
	{
		DeleteIndices();

		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();

		//Interior vertices all need 6 indices
		//limit vertices all need 3 indices
		//Except corner ones. Two need 1 and two need 2
		//numIndices = ((w - 2) * (h - 2)) * 6 + (w * 2 + h * 2) * 3 - 2 - 1 - 2 - 1;
		//indices = new uint[numIndices];

		float2* originalUvs = new float2[w*h];

		{
			BROFILER_CATEGORY("ModulePhysics3D::Generate_Indices::AddingTris", Profiler::Color::HoneyDew);
			for (int z = 0; z < h - 1; z++)
			{
				for (int x = 0; x < w - 1; x++)
				{
					AddTriToChunk(((z + 1) * w + x), (z * w + x + 1), (z * w + x), x, z);
					AddTriToChunk((z * w + x + 1), ((z + 1) * w + x), ((z + 1) * w + x + 1), x, z);

					originalUvs[z * w + x] = float2(((float)x / (float)w), (1 - ((float)z / (float)h)));
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
			glLineWidth(1.5f);
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
				glBindTexture(GL_TEXTURE_2D, textures[0]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[1]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[2]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[3]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[4]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[5]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[6]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[7]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[8]->GetTexture());
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
				glBindTexture(GL_TEXTURE_2D, textures[9]->GetTexture());
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
	if (terrainNormalBuffer != 0)
	{
		glDeleteBuffers(1, (GLuint*)&terrainNormalBuffer);
		terrainNormalBuffer = 0;
	}
}

void ModulePhysics3D::GenerateUVs()
{
	if (heightMapImg)
	{
		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();

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
	if (heightMapImg)
	{
		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();
		uint numVertices = w * h;

		DeleteVertices();

		vertices = new float3[numVertices];

		for (int z = 0; z < h; z++)
		{
			for (int x = 0; x < w; x++)
			{
				vertices[z * w + x] = float3(x - w / 2, realTerrainData[z * w + x] * terrainMaxHeight, z - h / 2);
			}
		}

		//Load vertices buffer to VRAM
		glGenBuffers(1, (GLuint*)&(terrainVerticesBuffer));
		glBindBuffer(GL_ARRAY_BUFFER, terrainVerticesBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * numVertices, vertices, GL_STATIC_DRAW);

		GenerateNormals();
	}
}

void ModulePhysics3D::DeleteVertices()
{
	if (terrainVerticesBuffer != 0)
	{
		glDeleteBuffers(1, (GLuint*)&terrainVerticesBuffer);
		terrainVerticesBuffer = 0;
	}
	if (vertices != nullptr)
	{
		delete[] vertices;
		vertices = nullptr;
	}
}

void ModulePhysics3D::GenerateNormals()
{
	if (heightMapImg && vertices)
	{
		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();
		uint numVertices = w * h;

		DeleteNormals();

		float3* normals = new float3[numVertices];

		for (int z = 0; z < h; z++)
		{
			for (int x = 0; x < w; x++)
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
				if (x + 1 < w && z - 1 > 0)
				{
					t.a = vertices[(z)* w + x];
					t.b = vertices[(z)* w + x + 1];
					t.c = vertices[(z - 1)* w + x];
					norm += t.NormalCCW();
				}
				//Bottom left
				if (x - 1 > 0 && z + 1 < h)
				{
					t.a = vertices[(z)* w + x];
					t.b = vertices[(z)* w + x - 1];
					t.c = vertices[(z + 1)* w + x];
					norm += t.NormalCCW();
				}
				//Bottom right
				if (x + 1 < w && z + 1 < h)
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

		//Load Normalss -----------------------------------------------------------------------------------------------------------------------
		glGenBuffers(1, (GLuint*)&(terrainNormalBuffer));
		glBindBuffer(GL_ARRAY_BUFFER, terrainNormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * numVertices, normals, GL_STATIC_DRAW);

		delete[] normals;
	}
}

void ModulePhysics3D::InterpretHeightmapRGB(float * R, float * G, float * B)
{
	if (heightMapImg)
	{
		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();

		float* edgeH = new float[(w*h) * 3];
		float* edgeV = new float[(w*h) * 3];

		if (textureMap != nullptr)
		{
			delete[] textureMap;
		}
		textureMap = new float[w*h];

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

				textureMap[(h - y - 1) * w + x] = math::Sqrt(edgeH[y * w + x] * edgeH[y * w + x] + edgeV[y * w + x] * edgeV[y * w + x]);
				if (textureMap[(h - y - 1) * w + x] > maxVal)
				{
					maxVal = textureMap[(h - y - 1) * w + x];
				}
#pragma endregion
			}
		}

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				textureMap[y * w + x] /= maxVal;
			}
		}

		ReinterpretTextureMap();

		delete[] edgeH;
		delete[] edgeV;
		delete[] buf;
	}
}

void ModulePhysics3D::SetTerrainMaxHeight(float height)
{
	if (height > 0.1f)
	{
		if (heightMapImg)
		{
			for (unsigned int n = 0; n < heightMapImg->GetWidth() * heightMapImg->GetHeight(); n++)
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

void ModulePhysics3D::LoadTexture(string resLibPath, int pos)
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
					textures.push_back((ResourceFileTexture*)res);
				}
				else
				{
					textures[pos]->Unload();
					textures[pos] = (ResourceFileTexture*)res;
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
		for (std::vector<ResourceFileTexture*>::iterator it = textures.begin(); it != textures.end(); it++)
		{
			if (x == n)
			{
				textures[n]->Unload();
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

uint ModulePhysics3D::GetCurrentTerrainUUID()
{
	if (heightMapImg)
	{
		return heightMapImg->GetUUID();
	}
	return 0;
}

const char *ModulePhysics3D::GetHeightmapPath()
{
	if (heightMapImg)
	{
		return heightMapImg->GetFile();
	}
	char ret[5] = " ";
	return ret;
}

int ModulePhysics3D::GetHeightmap()
{
	if (heightMapImg != nullptr)
	{
		return heightMapImg->GetTexture();
	}
	return 0;
}

int ModulePhysics3D::GetTexture(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n]->GetTexture();
	}
	return 0;
}

uint ModulePhysics3D::GetTextureUUID(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n]->GetUUID();
	}
	return 0;
}

const char * ModulePhysics3D::GetTexturePath(uint n)
{
	if (n >= 0 && n < textures.size())
	{
		return textures[n]->GetFile();
	}
	char ret[5] = " ";
	return ret;
}

uint ModulePhysics3D::GetNTextures()
{
	return textures.size();
}

float2 ModulePhysics3D::GetHeightmapSize()
{
	if (heightMapImg)
	{
		return float2(heightMapImg->GetWidth(), heightMapImg->GetHeight());
	}
	return float2::zero;
}

void ModulePhysics3D::AutoGenerateTextureMap()
{
	if (heightMapImg)
	{
		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				if (textureMap[y * w + x] > 0.15)
				{
					textureMap[y * w + x] = 0.15f;
				}
				else
				{
					textureMap[y * w + x] = 0.05f;
				}
			}
		}
		ReinterpretTextureMap();
	}
}

void ModulePhysics3D::ReinterpretTextureMap()
{
	if (heightMapImg)
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, heightMapImg->GetWidth(), heightMapImg->GetHeight(), 0, GL_RED, GL_FLOAT, textureMap);
	}
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
	if (indices.empty() == false)
	{
		if (indices_bufferID == 0)
		{
			glGenBuffers(1, (GLuint*) &(indices_bufferID));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_bufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), indices.data(), GL_STATIC_DRAW);

		indices.shrink_to_fit();
	}
}

int chunk::GetBuffer()
{
	return indices_bufferID;
}

int chunk::GetNIndices()
{
	return indices.size();
}

void chunk::AddIndex(const uint& i)
{
	indices.push_back(i);
}

void chunk::UpdateAABB()
{
	aabb.SetNegativeInfinity();
	for (std::vector<uint>::iterator it = indices.begin(); it != indices.end(); it++)
	{
		uint a = (*it);
		aabb.Enclose(App->physics->vertices[(*it)]);
	}
}

void chunk::CleanIndices()
{
	indices.clear();
}

void chunk::Render()
{
	App->renderer3D->DrawAABB(aabb.minPoint, aabb.maxPoint, float4(0.674, 0.784, 0.886, 1.0f));
}
