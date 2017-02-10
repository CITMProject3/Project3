#include "StackAllocator.h"

StackAllocator::StackAllocator(size_t _size, void* _start) : Allocator(_size, _start), current_pos(_start)
{
	assert(_size > 0);

	#if _DEBUG
	prev_position = nullptr;
	#endif
}

StackAllocator::~StackAllocator()
{
	#if _DEBUG	
	prev_position = nullptr;
	#endif

	current_pos = nullptr;
}

void* StackAllocator::Allocate(size_t _size, uint8_t alignment)
{
	assert(_size != 0);

	uint8_t adjustment = allocator_pointer::AlignForwardAdjustmentWithHeader(current_pos, alignment, sizeof(AllocationHeader));

	if (used_memory + adjustment + _size > size)
		return nullptr;

	void* aligned_address = allocator_pointer::Add(current_pos, adjustment);

	AllocationHeader* header = (AllocationHeader*)(allocator_pointer::Subtract(aligned_address, sizeof(AllocationHeader)));

	header->adjustment = adjustment;

	#if _DEBUG
	header->prev_address = prev_position;
	prev_position = aligned_address;
	#endif

	current_pos = allocator_pointer::Add(aligned_address, size);

	used_memory += size + adjustment;
	num_allocations++;

	return aligned_address;
}

void StackAllocator::Deallocate(void* p)
{
	#if _DEBUG
	assert(p == prev_position);
	#endif _DEBUG

	AllocationHeader* header = (AllocationHeader*)(allocator_pointer::Subtract(p, sizeof(AllocationHeader)));

	used_memory -= (uintptr_t)current_pos - (uintptr_t)p + header->adjustment;

	current_pos = allocator_pointer::Subtract(p, header->adjustment);

	#if _DEBUG
	prev_position = header->prev_address;
	#endif

	num_allocations--;
}