#ifndef __COMPONENTMESH_H__
#define __COMPONENTMESH_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "Globals.h"

struct Bone_Vertex
{
	//All bones that influence the vertex
	//stored in "bones_reference" vector
	std::vector<uint> bone_index;
	std::vector<float> weights;

	void AddBone(uint index, float weight){bone_index.push_back(index);	weights.push_back(weight);}
};

class ComponentBone;

struct Bone_Reference
{
	Bone_Reference(ComponentBone* bone, float4x4 offset) { this->bone = bone; this->offset = offset; }
	ComponentBone* bone;
	float4x4 offset = float4x4::identity;
};

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
class ComponentBone;

class ComponentMesh : public Component
{
public:

	ComponentMesh(ComponentType type, GameObject* game_object);
	~ComponentMesh();

	void Update();

	void OnInspector(bool debug);
	void OnTransformModified();

	bool SetMesh(Mesh* mesh);
	void SetResourceMesh(ResourceFileMesh* resource);
	void RecalculateBoundingBox();

	void Save(Data& file) const;
	void Load(Data& conf);

	const Mesh* GetMesh()const { return mesh; };
	ResourceFileMesh* GetResource() const { return rc_mesh; };

	void Remove();

	bool HasBones();
	void AddBone(ComponentBone* bone);
	void DeformAnimMesh();

	AABB GetBoundingBox() { return bounding_box; }
	AABB GetLocalAABB() { return aabb; }

	void InitAnimBuffers();

private:

	ResourceFileMesh* rc_mesh = nullptr;
	Mesh* mesh = nullptr;

	std::vector<Bone_Reference> bones_reference;
	std::vector<Bone_Vertex> bones_vertex;

	math::AABB aabb; //Local one
	math::AABB bounding_box; //In the world position

public:

	bool animated = false;
	unsigned int weight_id = 0;
	unsigned int bone_id = 0;
	std::vector<math::float4x4> bones_trans;
};


#endif // !__COMPONENTMESH_H__
