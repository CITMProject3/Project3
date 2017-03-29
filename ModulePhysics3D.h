#ifndef __MODULEPHYSICS_H__
#define __MODULEPHYSICS_H__

#include "Module.h"

#include "Primitive.h"

#include <list>
#include <string>

#include "Bullet\include\btBulletDynamicsCommon.h"
#include "Bullet\include\btBulletCollisionCommon.h"

// Recommended scale is 1.0f == 1 meter, no less than 0.2 objects
#define GRAVITY btVector3(0.0f, -10.0f, 0.0f) 

class PhysBody3D;
struct PhysVehicle3D;
struct VehicleInfo;

class DebugDrawer;
class ComponentMesh;
class ResourceFileTexture;
class ComponentCollider;
class ComponentCar;

class btHeightfieldTerrainShape;

class ModulePhysics3D : public Module
{
public:
	ModulePhysics3D(const char* name, bool start_enabled = true);
	~ModulePhysics3D();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void OnCollision(PhysBody3D* car, PhysBody3D* body);

	void OnPlay();
	void OnStop();

	void CleanWorld();
	void CreateGround();

	PhysBody3D* AddBody(const Sphere_P& sphere, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0);
	PhysBody3D* AddBody(const Cube_P& cube, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0);
	PhysBody3D* AddBody(const Cylinder_P& cylinder, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0);
	PhysBody3D* AddBody(const ComponentMesh& mesh, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0, btConvexHullShape** OUT_shape = nullptr);
	PhysVehicle3D* AddVehicle(const VehicleInfo& info, ComponentCar* col);

	bool GenerateHeightmap(std::string resLibPath);
	void DeleteHeightmap();
	void SetTerrainHeightScale(float scale);

	void LoadTexture(std::string resLibPath);
	void DeleteTexture();

	bool TerrainIsGenerated();
	float GetTerrainHeightScale() { return terrainHeightScaling; }
	uint GetCurrentTerrainUUID();
	const char* GetHeightmapPath();
	int GetHeightmap();
	float2 GetHeightmapSize();

	int GetTexture();
	uint GetTextureUUID();
	const char* GetTexturePath();
	void RenderTerrain();
private:
	void AddTerrain();

	void GenerateIndices();
	
	void GenerateTerrainMesh();
	void DeleteTerrainMesh();
	void InterpretHeightmapRGB(float* R, float* G, float* B);
public:

	void AddConstraintP2P(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec& anchorA, const vec& anchorB);
	void AddConstraintHinge(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec& anchorA, const vec& anchorB, const vec& axisS, const vec& axisB, bool disable_collision = false);

private:

	bool debug = false;

	btDefaultCollisionConfiguration*	collision_conf;
	btCollisionDispatcher*				dispatcher;
	btBroadphaseInterface*				broad_phase;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld*			world;
	btDefaultVehicleRaycaster*			vehicle_raycaster;
	DebugDrawer*						debug_draw = nullptr;

	std::list<btCollisionShape*> shapes;
	std::list<PhysBody3D*> bodies;
	std::list<btDefaultMotionState*> motions;
	std::list<btTypedConstraint*> constraints;
	std::list<PhysVehicle3D*> vehicles;

#pragma region Terrain
	uint* indices = nullptr;
	float* terrainData = nullptr;
	btHeightfieldTerrainShape* terrain = nullptr;
	ResourceFileTexture* heightMapImg = nullptr;
	ResourceFileTexture* texture = nullptr;
	float terrainHeightScaling = 0.5f;

	int terrainVerticesBuffer = 0;
	int terrainIndicesBuffer = 0;
	int terrainUvBuffer = 0;
	int terrainNormalBuffer = 0;

	int terrainSmoothLevels = 1;
	uint numIndices = 0;
#pragma endregion
public:
	//TMP stuff
	uint edgeTexId = 0;
	float* edgeDetectionImage = nullptr;
	///////

	bool renderWiredTerrain = false;
};

class DebugDrawer : public btIDebugDraw
{
public:

	DebugDrawer() : line(0,0,0)
	{}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int	 getDebugMode() const;

	DebugDrawModes mode;
	Line_P line;
	Primitive point;
};

#endif // !__MODULEPHYSICS_H__
