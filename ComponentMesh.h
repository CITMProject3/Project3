#ifndef __COMPONENTMESH_H__
#define __COMPONENTMESH_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

struct Mesh
{
	//Vertices
	unsigned int id_vertices = 0;
	unsigned int num_vertices = 0;
	float* vertices = nullptr;

	//Indices
	unsigned int id_indices = 0;
	unsigned int num_indices = 0;
	unsigned int* indices = nullptr;

	//UVs
	unsigned int id_uvs = 0;
	unsigned int num_uvs = 0;
	float* uvs = nullptr;

	//Normals
	float* normals = nullptr;
	unsigned int id_normals = 0;

	//Tangent
	float* tangents = nullptr;
	unsigned int id_tangents = 0;

	//Vertex colors
	float* colors = nullptr;

	std::string file_path;
};

class ResourceFileMesh;

class ComponentMesh : public Component
{
public:
	ComponentMesh(ComponentType type, GameObject* game_object);
	~ComponentMesh();

	void Update();

	void OnInspector();
	void OnTransformModified();

	bool SetMesh(Mesh* mesh);
	void RecalculateBoundingBox();

	void Save(Data& file)const;
	void Load(Data& conf);

	const Mesh* GetMesh()const;

private:
	ResourceFileMesh* rc_mesh = nullptr;
	Mesh* mesh = nullptr;
	math::AABB aabb; //Local one
	math::AABB bounding_box; //In the world position
};


#endif // !__COMPONENTMESH_H__
