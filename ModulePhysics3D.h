#ifndef __MODULEPHYSICS_H__
#define __MODULEPHYSICS_H__

#include "Module.h"

#include "Primitive.h"

#include "RaycastHit.h"

#include <list>
#include <string>
#include <map>

#include "Bullet\include\btBulletDynamicsCommon.h"
#include "Bullet\include\btBulletCollisionCommon.h"

// Recommended scale is 1.0f == 1 meter, no less than 0.2 objects
#define GRAVITY btVector3(0.0f, -10.0f, 0.0f) 

#define MAX_TERRAIN_TEXTURES 10

#define CHUNK_W 64
#define CHUNK_H 64

class PhysBody3D;
struct PhysVehicle3D;
struct VehicleInfo;

class DebugDrawer;
class ComponentMesh;
class ResourceFileTexture;
class ComponentCollider;
class ComponentCar;
class ComponentCamera;

class btHeightfieldTerrainShape;

class chunk
{
public:
	chunk();
	~chunk();

	int GetBuffer();
	int GetNIndices();

	void GenBuffer();
	void AddIndex(const uint& i);

	void UpdateAABB();
	void CleanIndices();

	void Render();

	AABB GetAABB() { return aabb; }

	std::vector<uint> indices;
private:
	math::AABB aabb;	
	int indices_bufferID = 0;
};

class ModulePhysics3D : public Module
{
	friend class chunk;
public:
	ModulePhysics3D(const char* name, bool start_enabled = true);
	~ModulePhysics3D();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void GetShaderLocations();

	void OnCollision(PhysBody3D* car, PhysBody3D* body);

	void OnPlay();
	void OnStop();

	void CleanWorld();
	void CreateGround();

	bool RayCast(Ray ray, RaycastHit& hit);

	PhysBody3D* AddBody(const Sphere_P& sphere, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0);
	PhysBody3D* AddBody(const Cube_P& cube, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0);
	PhysBody3D* AddBody(const Cylinder_P& cylinder, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0);
	PhysBody3D* AddBody(const ComponentMesh& mesh, ComponentCollider* col, float mass = 1.0f, unsigned char flags = 0, btConvexHullShape** OUT_shape = nullptr);
	PhysVehicle3D* AddVehicle(const VehicleInfo& info, ComponentCar* col);

	bool GenerateHeightmap(std::string resLibPath);
	void DeleteHeightmap();
	void SetTerrainMaxHeight(float height);
	void SetTextureScaling(float scale, bool doNotUse = false);

	void LoadTexture(std::string resLibPath, int pos = -1);
	void DeleteTexture(uint n);

	bool SaveTextureMap(const char* path);
	void LoadTextureMap(const char* path);

	bool TerrainIsGenerated();
	float GetTerrainHeightScale() { return terrainMaxHeight; }
	uint GetCurrentTerrainUUID();
	const char* GetHeightmapPath();
	int GetHeightmap();
	float2 GetHeightmapSize();

	void AutoGenerateTextureMap();
	void ReinterpretTextureMap();

	int GetTexture(uint n);
	uint GetTextureUUID(uint n);
	const char* GetTexturePath(uint n);
	uint GetNTextures();
	float GetTextureScaling() { return textureScaling; }

	void RenderTerrain(ComponentCamera* camera);
private:
	void RealRenderTerrain(ComponentCamera* camera, bool wired = false);

	void AddTerrain();
	
	void GenerateTerrainMesh();
	void DeleteTerrainMesh();

	void GenerateVertices();
	void DeleteVertices();
	void GenerateNormals();
	void DeleteNormals();
	void GenerateUVs();
	void DeleteUVs();
	void GenerateIndices();
	void DeleteIndices();

	void GenerateChunks();
	void UpdateChunksAABBs();
	void AddTriToChunk(const uint& i1, const uint& i2, const uint& i3, int& x, int& z);

	std::vector<chunk> GetVisibleChunks(ComponentCamera* camera);

	int GetNChunksW() { return chunks[0].size(); }
	int GetNChunksH() { return chunks.size(); }

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
	//Ordering chunks. First map contains Y coordinate, second one X. Chunks[y][x]
	std::map<int, std::map<int, chunk>> chunks;

	float3* vertices = nullptr;
	float* terrainData = nullptr;
	float* realTerrainData = nullptr;
	btHeightfieldTerrainShape* terrain = nullptr;
	ResourceFileTexture* heightMapImg = nullptr;
	std::vector<ResourceFileTexture*> textures;
	float textureScaling = 0.03f;
	float terrainMaxHeight = 100.0f;

	int terrainVerticesBuffer = 0;
	int terrainUvBuffer = 0;
	int terrainOriginalUvBuffer = 0;
	int terrainNormalBuffer = 0;

	int terrainSmoothLevels = 1;

	//Shader locations. Saving all of this avoids having to find all them each frame
	uint shader_id = 0;

	int model_location = 0;
	int projection_location = 0;
	int view_location = 0;
	int n_texs_location = 0;
	int tex_distributor_location = 0;

	int texture_location_0 = 0;
	int texture_location_1 = 0;
	int texture_location_2 = 0;
	int texture_location_3 = 0;
	int texture_location_4 = 0;
	int texture_location_5 = 0;
	int texture_location_6 = 0;
	int texture_location_7 = 0;
	int texture_location_8 = 0;
	int texture_location_9 = 0;

	int has_tex_location = 0;
	int texture_location = 0;

	int colorLoc = 0;
	int ambient_intensity_location = 0;
	int ambient_color_location = 0;
	int has_directional_location = 0;

	int directional_intensity_location = 0;
	int directional_color_location = 0;
	int directional_direction_location = 0;


#pragma endregion
public:
	uint textureMapBufferID = 0;
	float* textureMap = nullptr;
	
	bool renderChunks = false;
	bool paintMode = false;
	int paintTexture = 0;
	int brushSize = 5;

	bool renderWiredTerrain = false;
	bool renderFilledTerrain = true;
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
