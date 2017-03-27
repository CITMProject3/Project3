#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <string>

class CubeMap;
struct Mesh;
class ComponentCamera;

#define SKYBOX_MESH_PATH "Resources/Skybox/skybox_sphere.msh"
#define SKYBOX_VERTEX_PROGRAM "Resources/Skybox/skybox_vertex.ver"
#define SKYBOX_FRAGMENT_PROGRAM "Resources/Skybox/skybox_fragment.fra"

using namespace std;

class Skybox
{
public:
	Skybox();
	~Skybox();

	void Init(const string& posx_filename, const string& negx_filename, const string& posy_filename, const string& negy_filename, const string& posz_filename, const string& negz_filename);
	void Render(ComponentCamera* cam);

private:
	ComponentCamera* camera = nullptr;
	CubeMap* cubemap = nullptr;
	Mesh* sphere_mesh = nullptr;
	unsigned int shader_id = 0;
};



#endif // !__SKYBOX_H__

