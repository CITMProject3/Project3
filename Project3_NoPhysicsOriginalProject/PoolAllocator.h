#ifndef __POOL_ALLOCATOR_H__
#define __POOL_ALLOCATOR_H__

#include "Allocator.h"

class PoolAllocator : public Allocator
{
public:
	PoolAllocator(size_t _object_size, uint8_t _object_alignment, size_t _size, void* mem);
	~PoolAllocator();

	void* Allocate(size_t _size, uint8_t alignment) override;

	void Deallocate(void* p) override;

private:
	PoolAllocator(const PoolAllocator&); //Prevent copies
	PoolAllocator& operator= (const PoolAllocator&);

	size_t object_size;
	uint8_t object_alignment;

	void** free_list;
};

#endif // !__POOL__ALLOCATOR_H__
