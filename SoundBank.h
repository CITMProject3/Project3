#ifndef __SOUNDBANK_H__
#define __SOUNDBANK_H__

#include <string>
#include <vector>

class AudioEvent;

class SoundBank
{

public:

	std::vector<AudioEvent*> events;

	std::string name;
	std::string path;
	unsigned int id = 0;
};

#endif //__SOUNDBANK_H__
