#ifndef __LINEAR_ALLOCATOR_H__
#define __LINEAR_ALLOCATOR_H__

#include "Allocator.h"

class LinearAllocator : public Allocator
{
public:
	LinearAllocator(size_t _size, void* _start);
	~LinearAllocator();

	void* Allocate(size_t _size, uint8_t alignment) override;

	void Deallocate(void* p) override;
	
	void Clear();

private:

	LinearAllocator(const LinearAllocator&);
	LinearAllocator& operator=(const LinearAllocator&);

	void* current_pos;
};

#endif // !__LINEAR_ALLOCATOR_H__
