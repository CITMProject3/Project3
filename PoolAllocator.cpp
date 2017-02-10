#include "PoolAllocator.h"

PoolAllocator::PoolAllocator(size_t _object_size, uint8_t _object_alignment, size_t _size, void* mem) : Allocator(_size, mem), object_size(_object_size), object_alignment(_object_alignment)
{
	assert(_object_size >= sizeof(void*));

	uint8_t adjustment = allocator_pointer::AlignForwardAdjustment(mem, _object_alignment);

	free_list = (void**)allocator_pointer::Add(mem, adjustment);

	size_t num_objects = (size - adjustment) / _object_size;

	void** p = free_list;

	for (size_t i = 0; i < num_objects - 1; i++)
	{
		*p = allocator_pointer::Add(p, _object_size);
		p = (void**)*p;
	}

	*p = nullptr;
}

PoolAllocator::~PoolAllocator()
{
	free_list = nullptr;
}

void* PoolAllocator::Allocate(size_t _size, uint8_t _alignment)
{
	assert(_size == object_size && _alignment == object_alignment);

	if (free_list == nullptr)
		return nullptr;

	void* p = free_list;

	free_list = (void**)(*free_list);

	used_memory += _size;
	num_allocations++;

	return p;
}

void PoolAllocator::Deallocate(void* p)
{
	*((void**)p) = free_list;

	free_list = (void**)p;

	used_memory -= object_size;
	num_allocations--;
}