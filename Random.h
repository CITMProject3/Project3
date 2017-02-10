#ifndef __RANDOM_H__
#define __RANDOM_H__
#include <stdint.h>
class Random
{
public:
	Random();
	float RandomFloat();
	int RandomInt();
	int RandomInt(int min, int max);

	void SetSeed(unsigned int seed);

private:
	unsigned int seed;
};
#endif // !__RANDOM_H__
