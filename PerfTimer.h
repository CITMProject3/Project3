#ifndef __PERFTIMER_H__
#define __PERFTIMER_H__

#include <cstdint>

class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void Start();
	double ReadMs() const;
	uint64_t ReadTicks() const;

private:
	uint64_t	started_at;
	static uint64_t frequency;
};

#endif //__PERFTIMER_H__