#ifndef __CUBEMAP_H__
#define __CUBEMAP_H__

#include <string>
#include <vector>

using namespace std;

class CubeMap
{
public:
	CubeMap(const string& posx_filename, const string& negx_filename, const string& posy_filename, const string& negy_filename, const string& posz_filename, const string& negz_filename);
	~CubeMap();

	bool Load();
	bool Unload();
	void Bind(int texture_unit);

private:
	unsigned int id = 0;
	vector<string> textures_filenames;
};

#endif // !__CUBEMAP_H__
