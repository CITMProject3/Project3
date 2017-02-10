#include "LinearAllocator.h"

LinearAllocator::LinearAllocator(size_t _size, void* _start) : Allocator(_size, _start), current_pos(_start)
{
	assert(_size > 0);
}

LinearAllocator::~LinearAllocator()
{
	current_pos = nullptr;
}

void* LinearAllocator::Allocate(size_t _size, uint8_t alignment)
{
	assert(_size != 0);

	uint8_t adjustment = allocator_pointer::AlignForwardAdjustment(current_pos, alignment);

	if (used_memory + adjustment + _size > size)
		return nullptr;

	uintptr_t aligned_address = (uintptr_t)current_pos + adjustment;

	current_pos = (void*)(aligned_address + _size);

	used_memory += _size + adjustment;
	num_allocations++;

	return (void*)aligned_address;
}

void LinearAllocator::Deallocate(void* p)
{
	assert(false); //Use Clear() instead
}

void LinearAllocator::Clear()
{
	num_allocations = 0;
	used_memory = 0;

	current_pos = start;
}