#include "Glew\include\glew.h"

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

	RenderTerrain();

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

bool ModulePhysics3D::GenerateHeightmap(string resLibPath)
{	

	bool ret = false;
	//Loading Heightmap Image
	ResourceFile* res = App->resource_manager->LoadResource(resLibPath, ResourceFileType::RES_TEXTURE);
	if (res != nullptr && res->GetType() == ResourceFileType::RES_TEXTURE)
	{
		if (heightMapImg != nullptr)
		{
			heightMapImg->Unload();
		}
		DeleteTerrain();
		heightMapImg = (ResourceFileTexture*)res;

		//If the file exists and is loaded succesfully, we need to reload it manually.
		//The Resource won't hold the pixel data
		//Don't worry, we'll be deleting soon
		char* buffer = nullptr;
		unsigned int size = App->file_system->Load(res->GetFile(), &buffer);
		if (size > 0)
		{
			ILuint id;
			ilGenImages(1, &id);
			ilBindImage(id);
			if (ilLoadL(IL_DDS, (const void*)buffer, size))
			{
				int width = ilGetInteger(IL_IMAGE_WIDTH);
				int height = ilGetInteger(IL_IMAGE_HEIGHT);
				BYTE* tmp = new BYTE[width * height * 3];
				//Copying all RGB data of each pixel into a uchar (BYTE) array. We need to transform it into float numbers
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
	return ret;
}

void ModulePhysics3D::DeleteTerrain()
{
	if (terrainData != nullptr)
	{
		delete[] terrainData;
		terrainData = nullptr;

		terrain = nullptr;
		if (heightMapImg != nullptr)
		{
			heightMapImg->Unload();
			heightMapImg = nullptr;
		}
		if (texture != nullptr)
		{
			texture->Unload();
			texture = nullptr;
		}
		if (terrainVerticesBuffer != 9)
		{
			glDeleteBuffers(1, (GLuint*)&terrainVerticesBuffer);
		}
		terrainVerticesBuffer = 0;

		if (terrainIndicesBuffer != 9)
		{
			glDeleteBuffers(1, (GLuint*)&terrainIndicesBuffer);
		}
		terrainIndicesBuffer = 0;

		if (terrainUvBuffer != 9)
		{
			glDeleteBuffers(1, (GLuint*)&terrainUvBuffer);
		}
		terrainUvBuffer = 0;

		if (terrainNormalBuffer != 9)
		{
			glDeleteBuffers(1, (GLuint*)&terrainNormalBuffer);
		}
		terrainNormalBuffer = 0;


		if (numIndices != 9)
		{
			glDeleteBuffers(1, (GLuint*)&numIndices);
		}
		numIndices = 0;
	}
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

void ModulePhysics3D::RenderTerrain()
{
	/*if (terrain)
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

		for (int z = 0; z < heightMapImg->GetHeight() - 1; z++)
		{
			for (int x = 0; x < heightMapImg->GetWidth() - 1; x++)
			{
				tri1.a = vec(x - heightMapImg->GetWidth()  / 2, terrainData[z * heightMapImg->GetWidth()  + x] * terrainHeightScaling, z - heightMapImg->GetHeight() / 2);
				tri1.b = vec(x + 1 - heightMapImg->GetWidth()  / 2, terrainData[z * heightMapImg->GetWidth()  + x + 1] * terrainHeightScaling, z - heightMapImg->GetHeight() / 2);
				tri1.c = vec(x - heightMapImg->GetWidth()  / 2, terrainData[(z + 1) * heightMapImg->GetWidth()  + x] * terrainHeightScaling, z + 1 - heightMapImg->GetHeight() / 2);

				glVertex3fv(tri1.c.ptr());
				glVertex3fv(tri1.b.ptr());
				glVertex3fv(tri1.a.ptr());
				glNormal3fv(tri1.NormalCW().ptr());

				//If we're using wireframe, we only need to render half of the triangles for the mesh to look complete
				if (renderWiredTerrain == false)
				{
					tri2.a = vec(x + 1 - heightMapImg->GetWidth()  / 2, terrainData[(z + 1) * heightMapImg->GetWidth()  + x + 1], z + 1 - heightMapImg->GetHeight() / 2);
					tri2.b = vec(x + 1 - heightMapImg->GetWidth()  / 2, terrainData[z * heightMapImg->GetWidth()  + x + 1], z - heightMapImg->GetHeight() / 2);
					tri2.c = vec(x - heightMapImg->GetWidth()  / 2, terrainData[(z + 1) * heightMapImg->GetWidth()  + x], z + 1 - heightMapImg->GetHeight() / 2);

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
	}*/



	if (numIndices != 0 && terrainData != nullptr)
	{
		if (renderWiredTerrain)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		uint shader_id = App->resource_manager->GetDefaultShaderId();
		//Use shader
		glUseProgram(shader_id);

		//Set uniforms

		//Matrices
		GLint model_location = glGetUniformLocation(shader_id, "model");
		glUniformMatrix4fv(model_location, 1, GL_FALSE, *(float4x4::identity).v);
		GLint projection_location = glGetUniformLocation(shader_id, "projection");
		glUniformMatrix4fv(projection_location, 1, GL_FALSE, *App->renderer3D->camera->GetProjectionMatrix().v);
		GLint view_location = glGetUniformLocation(shader_id, "view");
		glUniformMatrix4fv(view_location, 1, GL_FALSE, *App->renderer3D->camera->GetViewMatrix().v);

		int count = 0;
		if (texture != nullptr)
		{
			GLint has_tex_location = glGetUniformLocation(shader_id, "_HasTexture");
			glUniform1i(has_tex_location, 1);
			GLint texture_location = glGetUniformLocation(shader_id, "_Texture");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->GetTexture());
			glUniform1i(texture_location, 0);
		}
		else
		{
			GLint has_tex_location = glGetUniformLocation(shader_id, "_HasTexture");
			glUniform1i(has_tex_location, 0);
		}


		//Lighting
		LightInfo light = App->lighting->GetLightInfo();
		//Ambient
		GLint ambient_intensity_location = glGetUniformLocation(shader_id, "_AmbientIntensity");
		if (ambient_intensity_location != -1)
			glUniform1f(ambient_intensity_location, light.ambient_intensity);
		GLint ambient_color_location = glGetUniformLocation(shader_id, "_AmbientColor");
		if (ambient_color_location != -1)
			glUniform3f(ambient_color_location, light.ambient_color.x, light.ambient_color.y, light.ambient_color.z);

		//Directional
		GLint has_directional_location = glGetUniformLocation(shader_id, "_HasDirectional");
		glUniform1i(has_directional_location, light.has_directional);

		if (light.has_directional)
		{
			GLint directional_intensity_location = glGetUniformLocation(shader_id, "_DirectionalIntensity");
			if (directional_intensity_location != -1)
				glUniform1f(directional_intensity_location, light.directional_intensity);
			GLint directional_color_location = glGetUniformLocation(shader_id, "_DirectionalColor");
			if (directional_color_location != -1)
				glUniform3f(directional_color_location, light.directional_color.x, light.directional_color.y, light.directional_color.z);
			GLint directional_direction_location = glGetUniformLocation(shader_id, "_DirectionalDirection");
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
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
/*
		//Buffer tangents == 3
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);*/

		//Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIndicesBuffer);
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glBindTexture(GL_TEXTURE_2D, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void ModulePhysics3D::GenerateTerrainMesh()
{
	if (heightMapImg)
	{
		int w = heightMapImg->GetWidth();
		int h = heightMapImg->GetHeight();
		uint numVertices = w * h;
		numIndices = ((w - 2) * (h - 2)) * 6 + (w * 2 * 3) + (h * 2 * 3) - 2 - 1 - 2 - 1;

		uint* indices = new uint[numIndices];

		float3* vertices = new float3[numVertices];
		float3* normals = new float3[numVertices];
		float2* uvs = new float2[numVertices];

		for (int z = 0; z < h; z++)
		{
			for (int x = 0; x < w; x++)
			{
				vertices[z * w + x] = float3(x - w / 2, terrainData[z * w + x], z - h / 2);
				float uv_x = (float)x / (float)w;
				float uv_y = 1 - ((float)z / (float)h);
				uvs[z * w + x] = float2(uv_x, uv_y);
			}
		}

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

		int n = 0;
		for (int z = 0; z < h - 1; z++)
		{
			for (int x = 0; x < w - 1; x++)
			{
				indices[n] = (z + 1) * w + x;
				n++;
				indices[n] = z * w + x + 1;
				n++;
				indices[n] = z * w + x;
				n++;

				indices[n] = z * w + x + 1;
				n++;
				indices[n] = (z + 1) * w + x;
				n++;
				indices[n] = (z + 1) * w + x + 1;
				n++;
			}
		}

		//Load vertices buffer to VRAM
		glGenBuffers(1, (GLuint*)&(terrainVerticesBuffer));
		glBindBuffer(GL_ARRAY_BUFFER, terrainVerticesBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * numVertices, vertices, GL_STATIC_DRAW);

		//Load UVs -----------------------------------------------------------------------------------------------------------------------
		glGenBuffers(1, (GLuint*)&(terrainUvBuffer));
		glBindBuffer(GL_ARRAY_BUFFER, terrainUvBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * numVertices, uvs, GL_STATIC_DRAW);

		//Load UVs -----------------------------------------------------------------------------------------------------------------------
		glGenBuffers(1, (GLuint*)&(terrainNormalBuffer));
		glBindBuffer(GL_ARRAY_BUFFER, terrainNormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * numVertices, normals, GL_STATIC_DRAW);

		//Load indices buffer to VRAM
		glGenBuffers(1, (GLuint*) &(terrainIndicesBuffer));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIndicesBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * numIndices, indices, GL_STATIC_DRAW);


		delete[] indices;
		delete[] vertices;
		delete[] normals;
		delete[] uvs;
	}
}

void ModulePhysics3D::DeleteTerrainMesh()
{
}

void ModulePhysics3D::InterpretHeightmapRGB(float * R, float * G, float * B)
{
	int w = heightMapImg->GetWidth();
	int h = heightMapImg->GetHeight();

	float* Rbuf = new float[w*h];
	float* Gbuf = new float[w*h];
	float* Bbuf = new float[w*h];

	//Iterating all image pixels
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			float value = 0.0f;
			int n = 0;
			int displacement = 0;
			//Iterating all nearby pixels and checking they actually exist in the image
			for (int _y = y - displacement; _y <= y + displacement; _y++)
			{
				if (_y > 0 && _y < h)
				{
					for (int _x = x - displacement; _x <= x + displacement; _x++)
					{
						if (_x > 0 && _x < w)
						{
							n++;
							value += R[_y * w + _x];
						}
					}
				}
			}
			value /= n;
			Rbuf[y*w + x] = value;

			value = 0.0f;
			n = 0;
			displacement = 2;
			//Iterating all nearby pixels and checking they actually exist in the image
			for (int _y = y - displacement; _y <= y + displacement; _y++)
			{
				if (_y > 0 && _y < h)
				{
					for (int _x = x - displacement; _x <= x + displacement; _x++)
					{
						if (_x > 0 && _x < w)
						{
							n++;
							value += G[_y * w + _x];
						}
					}
				}
			}
			value /= n;
			Gbuf[y*w + x] = value;

			value = 0.0f;
			n = 0;
			displacement = 1;
			for (int _y = y - displacement; _y <= y + displacement; _y++)
			{
				if (_y > 0 && _y < h)
				{
					for (int _x = x - displacement; _x <= x + displacement; _x++)
					{
						if (_x > 0 && _x < w)
						{
							n++;
							value += B[_y * w + _x];
						}
					}
				}
			}
			value /= n;
			Bbuf[y*w + x] = value;

		}
	}

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			terrainData[y*w + x] = (max(max(Rbuf[y*w + x], Gbuf[y*w + x]), Bbuf[y*w + x]) / 3.0) * terrainHeightScaling;
		}
	}

	delete[] Rbuf;
	delete[] Gbuf;
	delete[] Bbuf;
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

void ModulePhysics3D::SetTerrainHeightScale(float scale)
{
	if (scale != 0)
	{
		if (heightMapImg)
		{
			for (int n = 0; n < heightMapImg->GetWidth() * heightMapImg->GetHeight(); n++)
			{
				terrainData[n] = (terrainData[n] / terrainHeightScaling) * scale;
			}
		}
	}
	terrainHeightScaling = scale;
	GenerateTerrainMesh();
}

void ModulePhysics3D::LoadTexture(string resLibPath)
{
	bool ret = false;
	//Loading Heightmap Image
	ResourceFile* res = App->resource_manager->LoadResource(resLibPath, ResourceFileType::RES_TEXTURE);
	if (res != nullptr && res->GetType() == ResourceFileType::RES_TEXTURE)
	{
		if (texture != nullptr)
		{
			texture->Unload();
		}
		texture = (ResourceFileTexture*)res;
	}
}

uint ModulePhysics3D::GetCurrentTerrainUUID()
{
	return heightMapImg->GetUUID();
}

int ModulePhysics3D::GetHeightmap()
{
	if (heightMapImg != nullptr)
	{
		return heightMapImg->GetTexture();
	}
	return 0;
}

int ModulePhysics3D::GetTexture()
{
	if (texture != nullptr)
	{
		return texture->GetTexture();
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
