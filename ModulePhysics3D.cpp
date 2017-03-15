#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "PhysBody3D.h"
#include "PhysVehicle3D.h"
#include "Primitive.h"
#include "ComponentMesh.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Assets.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ComponentCamera.h"

#include "ModuleRenderer3D.h"
#include "ModuleGOManager.h"
#include "LayerSystem.h"

#include "Devil/include/il.h"
#include "Devil/include/ilut.h"

#include "Bullet\include\BulletCollision\CollisionShapes\btShapeHull.h"
#include "Bullet\include\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"

#include "BtTriProcessor.h"
#include "RaycastHit.h"

#include "ResourceFileTexture.h"

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
	debug_draw = new DebugDrawer(); //DEBUG DISABLED

	terrainSize.push_back(0);
	terrainSize.push_back(0);
	terrainSize.push_back(0);
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

	App->go_manager->layer_system->AddLayer(TERRAIN_LAYER, "Terrain");

	App->file_system->GenerateDirectory("Library/Terrains");

	//CreateGround();
	return true;
}

update_status ModulePhysics3D::PreUpdate()
{
	float dt = time->DeltaTime();
	if (App->IsGameRunning())
	{
		world->stepSimulation(dt, 15);
	}

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			PhysBody3D* pbodyA = (PhysBody3D*)obA->getUserPointer();
			PhysBody3D* pbodyB = (PhysBody3D*)obB->getUserPointer();

			if(pbodyA && pbodyB)
			{
				list<Module*>::iterator item = pbodyA->collision_listeners.begin();

				while (item != pbodyA->collision_listeners.end())
				{
					(*item)->OnCollision(pbodyA, pbodyB);
					++item;
				}

				item = pbodyB->collision_listeners.begin();
				while(item != pbodyB->collision_listeners.end())
				{
					(*item)->OnCollision(pbodyB, pbodyA);
					++item;
				}
			}
		}
	}

	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::Update()
{
	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(debug == true)
	{
		world->debugDrawWorld();
	}

	ContinuousTerrainGeneration();

	if (renderTerrain)
	{
		RenderTerrain();
	}

	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");

	CleanWorld();

	DeleteTerrain();

	delete vehicle_raycaster;
	delete world;

	return true;
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
		delete *item;

	bodies.clear();

	for (list<PhysVehicle3D*>::iterator item = vehicles.begin(); item != vehicles.end(); item++)
	{
		world->removeVehicle((*item)->vehicle);
		delete *item;
	}

	vehicles.clear();

	world->clearForces();

	//CreateGround();
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

void ModulePhysics3D::GenerateTerrain()
{
	if (terrainData != nullptr)
	{
		delete terrainData;
		terrainData = nullptr;
	}

	AABB terrainAABB = App->go_manager->GetWorldAABB(std::vector<int>(1, TERRAIN_LAYER));
	int X = max(Abs(terrainAABB.minPoint.x), Abs(terrainAABB.maxPoint.x));
	int Z = max(Abs(terrainAABB.minPoint.z), Abs(terrainAABB.maxPoint.z));
	terrainSize[0] = X*2;
	terrainSize[1] = max(math::Abs(terrainAABB.minPoint.y), math::Abs(terrainAABB.maxPoint.y)) * 2;
	terrainSize[2] = Z*2;

	if (terrainSize[0] > 0 && terrainSize[2] > 0)
	{
		terrainData = new float[(terrainSize[0]) * (terrainSize[2])];
		x = 0;
		z = 0;
		loadingTerrain = true;
		loadInSecondPlane = false;
	}
}

bool ModulePhysics3D::GenerateHeightmap(string resLibPath)
{	
	ResourceFile* res = App->resource_manager->LoadResource(resLibPath, ResourceFileType::RES_TEXTURE);
	if (res != nullptr && res->GetType() == ResourceFileType::RES_TEXTURE)
	{
		heightMapImg = (ResourceFileTexture*)res;
		heightMapImg->Load();
		unsigned char* buf = heightMapImg->GetData();
		float* f = (float*)buf;
		uint* u = (uint*)buf;
		int h = heightMapImg->GetHeight();
		int w = heightMapImg->GetWidth();
		int size = h * w;
	}
	return false;
}

void ModulePhysics3D::DeleteTerrain()
{
	/*if (terrainData != nullptr)
	{
		delete terrainData;
		terrainData = nullptr;
		currentTerrainUUID = 0;
		terrainSize[0] = terrainSize[1] = terrainSize[2] = 0;
	}*/
}

bool ModulePhysics3D::TerrainIsGenerated()
{
	return (terrainData != nullptr);
}


// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Sphere_P& sphere, float mass, bool isSensor)
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
	PhysBody3D* pbody = new PhysBody3D(body);

	if (isSensor)
		body->setCollisionFlags(body->getCollisionFlags() |btCollisionObject::CF_NO_CONTACT_RESPONSE);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

PhysBody3D* ModulePhysics3D::AddBody(const Cube_P& cube, float mass, bool isSensor)
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
	PhysBody3D* pbody = new PhysBody3D(body);

	if (isSensor)
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

PhysBody3D* ModulePhysics3D::AddBody(const Cylinder_P& cylinder, float mass, bool isSensor)
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
	PhysBody3D* pbody = new PhysBody3D(body);

	if (isSensor)
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

PhysBody3D* ModulePhysics3D::AddBody(const ComponentMesh& mesh, float mass, bool isSensor, btConvexHullShape** out_shape)
{
	btConvexHullShape* colShape = new btConvexHullShape();

	float3* vertices = (float3*)mesh.GetMeshData()->vertices;
	uint nVertices = mesh.GetMeshData()->num_vertices;

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
	float4x4 go_trs = ((ComponentTransform*)(mesh.GetGameObject()->GetComponent(C_TRANSFORM)))->GetGlobalMatrix().Transposed();
	startTransform.setFromOpenGLMatrix(go_trs.ptr());

	btVector3 localInertia(0, 0, 0);
	if (mass != 0.f)
		simplifiedColShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, simplifiedColShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	if (isSensor)
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	delete hull;
	delete colShape;

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}


// ---------------------------------------------------------
PhysVehicle3D* ModulePhysics3D::AddVehicle(const VehicleInfo& info)
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

	PhysVehicle3D* pvehicle = new PhysVehicle3D(body, vehicle, info);
	world->addVehicle(vehicle);
	vehicles.push_back(pvehicle);

	pvehicle->SetTransform(info.transform.Transposed().ptr());

	return pvehicle;
}


// ---------------------------------------------------------
void ModulePhysics3D::AddTerrain()
{
	float minMax = terrainSize[1]/2;
	minMax += 1;

	if (terrainSize[0] > 0 && terrainSize[2] > 0 && terrainData != nullptr)
	{
		terrain = new btHeightfieldTerrainShape(terrainSize[0], terrainSize[2], terrainData, 1.0f, -minMax, minMax, 1, PHY_ScalarType::PHY_FLOAT, false);
		shapes.push_back(terrain);

		btDefaultMotionState* myMotionState = new btDefaultMotionState();
		motions.push_back(myMotionState);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, terrain);

		btRigidBody* body = new btRigidBody(rbInfo);

		world->addRigidBody(body);
	}
}

void ModulePhysics3D::ContinuousTerrainGeneration()
{
	if (loadingTerrain == true)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 180));
		if (ImGui::Begin("Generating terrain data"))
		{
			float fraction = ((float)(z * terrainSize[0] + x) / (float)(terrainSize[0] * terrainSize[2]));
			ImGui::ProgressBar(fraction);
			char str[124];
			sprintf(str, "Gathering height data.\n%i / %i\nModifying objects or entering Play mode may cause errors.", z * terrainSize[0] + x, terrainSize[0] * terrainSize[2]);
			ImGui::Text(str);
			ImGui::NewLine();
			ImGui::Checkbox("Second Plane Terrain Generation", &loadInSecondPlane);
			ImGui::Text("Marking this checkbox will make the terrain generation go slower,\n increasing the FPS");
			ImGui::End();
		}
		Timer time;
		time.Start();
		int n = 0;

		math::Ray ray;
		RaycastHit hit;
		float timerTime;
		for (; z < terrainSize[2]; z++)
		{
			for (; x < terrainSize[0]; x++)
			{				
				ray.dir = vec(0, -1, 0);
				ray.pos = vec(x - (terrainSize[0] / 2), terrainSize[1] + 3, z - (terrainSize[2] / 2));
				hit = App->go_manager->Raycast(ray, std::vector<int>(1, TERRAIN_LAYER));

				if (hit.object != nullptr)
				{
					terrainData[z * terrainSize[0] + x] = hit.point.y;
				}
				else
				{
					terrainData[z * terrainSize[0] + x] = 0;
				}
				n++;
				if (n % 5 == 0 || loadInSecondPlane)
				{
					timerTime = time.ReadSec();
					if ((timerTime > 0.15f) || (loadInSecondPlane && timerTime > 0.016f))
					{
						return;
					}
					n = 0;
				}
			}
			x = 0;
		}
		loadingTerrain = false;
		SaveTerrain();
	}
}

void ModulePhysics3D::RenderTerrain()
{
	if (terrain)
	{
		BtTriPRocessor tmp;
		tmp.useWire = renderWiredTerrain;
		AABB cullCam;
		cullCam.Enclose(App->renderer3D->GetCamera()->GetFrustum());
		terrain->processAllTriangles(&tmp, btVector3(cullCam.minPoint.x, cullCam.minPoint.y, cullCam.minPoint.z), btVector3(cullCam.maxPoint.x, cullCam.maxPoint.y, cullCam.maxPoint.z));
	}
	else if (terrainData)
	{
		if (renderWiredTerrain)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glBegin(GL_TRIANGLES);
		glColor3f(0.0f, 0.6f, 0.6f);
		Triangle tri1;
		Triangle tri2;

		for (int z = 0; z < terrainSize[2] - 1; z++)
		{
			for (int x = 0; x < terrainSize[0] - 1; x++)
			{
				tri1.a = vec(x - terrainSize[0] / 2, terrainData[z * terrainSize[0] + x], z - terrainSize[2] / 2);
				tri1.b = vec(x + 1 - terrainSize[0] / 2, terrainData[z * terrainSize[0] + x + 1], z - terrainSize[2] / 2);
				tri1.c = vec(x - terrainSize[0] / 2, terrainData[(z + 1) * terrainSize[0] + x], z + 1 - terrainSize[2] / 2);

				glVertex3fv(tri1.c.ptr());
				glVertex3fv(tri1.b.ptr());
				glVertex3fv(tri1.a.ptr());
				glNormal3fv(tri1.NormalCW().ptr());

				//If we're using wireframe, we only need to render half of the triangles for the mesh to look complete
				if (renderWiredTerrain == false)
				{
					tri2.a = vec(x + 1 - terrainSize[0] / 2, terrainData[(z + 1) * terrainSize[0] + x + 1], z + 1 - terrainSize[2] / 2);
					tri2.b = vec(x + 1 - terrainSize[0] / 2, terrainData[z * terrainSize[0] + x + 1], z - terrainSize[2] / 2);
					tri2.c = vec(x - terrainSize[0] / 2, terrainData[(z + 1) * terrainSize[0] + x], z + 1 - terrainSize[2] / 2);

					glVertex3fv(tri2.a.ptr());
					glVertex3fv(tri2.b.ptr());
					glVertex3fv(tri2.c.ptr());
					glNormal3fv(tri2.NormalCCW().ptr());
				}
			}
		}
		glEnd();
		if (renderWiredTerrain)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

bool ModulePhysics3D::SaveTerrain()
{
	bool ret = true;

	/*float* buf = new float[terrainSize[0] * terrainSize[2] + 3];
	float* it = buf;
	uint bytes = sizeof(int);
	memcpy(it, &terrainSize[0], bytes);
	it ++;
	memcpy(it, &terrainSize[1], bytes);
	it++;
	memcpy(it, &terrainSize[2], bytes);
	it ++;
	bytes = sizeof(float);
	memcpy(it, terrainData, bytes * terrainSize[0] * terrainSize[2]);

	string path = "Library/Terrains/";
	char tmp[24];
	sprintf(tmp, "%u", heightMapImg->GetUUID());
	path += tmp;
	path += ".trr";

	
	if (App->file_system->Save(path.data(), buf, terrainSize[0] * terrainSize[2] + 3) <= 0)
	{
		ret = false;
		LOG("Error saving terrain.");
	}

	delete[] buf;*/
	return ret;
}

bool ModulePhysics3D::LoadTerrain(uint uuid)
{
	bool ret = false;
	/*if (uuid != 0)
	{
		currentTerrainUUID = uuid;

		string path = "Library/Terrains/";
		char tmp[24];
		sprintf(tmp, "%u", uuid);
		path += tmp;
		path += ".trr";

		char* buf = nullptr;
		char* it = buf;
		App->file_system->Load(path.data(), &buf);

		if (buf != nullptr)
		{
			uint bytes = sizeof(float);
			memcpy(&terrainSize[0], it, bytes);
			it++;
			memcpy(&terrainSize[1], it, bytes);
			it++;
			memcpy(&terrainSize[2], it, bytes);
			it++;

			terrainData = new float[terrainSize[0] * terrainSize[2]];

			memcpy(terrainData, it, bytes * terrainSize[0] * terrainSize[2]);

			ret = true;
			delete[] buf;
		}
	}*/
	return ret;
}

uint ModulePhysics3D::GetCurrentTerrainUUID()
{
	return heightMapImg->GetUUID();
}

int ModulePhysics3D::GetHeightmapTexture()
{
	if (heightMapImg != nullptr)
	{
		return heightMapImg->GetTexture();
	}
	return 0;
}

float2 ModulePhysics3D::GetHeightmapSize()
{
	return float2(heightMapImg->GetWidth(), heightMapImg->GetHeight());
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
