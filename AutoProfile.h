#ifndef __AUTOPROFILE_H__
#define __AUTOPROFILE_H__

#include <cstdint>

class AutoProfile
{
public:
	AutoProfile(const char* name);
	~AutoProfile();
public:
	const char* name;
	uint64_t start_time;
private:
	static uint64_t frequency;
};

#define PROFILE(name) AutoProfile ap(name)

#endif // !__AUTOPROFILE_H__
