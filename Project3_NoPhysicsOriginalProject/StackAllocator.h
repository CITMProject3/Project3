#ifndef __STACK_ALLOCATOR_H__
#define __STACK_ALLOCATOR_H__

#include "Allocator.h"

class StackAllocator : public Allocator
{
public:
	StackAllocator(size_t _size, void* _start);
	~StackAllocator();

	void* Allocate(size_t _size, uint8_t alignment) override;
	void Deallocate(void* p) override;

private:
	StackAllocator(const StackAllocator&); //Prevent copies
	StackAllocator& operator= (const StackAllocator&);

	struct AllocationHeader
	{
		#if _DEBUG
		void* prev_address;
		#endif

		uint8_t adjustment;
	};

	#if _DEBUG
	void* prev_position;
	#endif

	void* current_pos;
};

#endif // !__STACK_ALLOCATOR_H__
