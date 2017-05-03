// -----------------------------
// Base class for all allocators
//-------------------------------
#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <assert.h>
#include <cstdint>

class Allocator
{
public:
	Allocator(size_t _size, void* _start)
	{
		start = _start;
		size = _size;
		used_memory = 0;
		num_allocations = 0;
	}

	virtual ~Allocator()
	{
		assert(num_allocations == 0 && used_memory == 0); //Memory leak detection
		start = nullptr;
		size = 0;
	}

	virtual void* Allocate(size_t _size, uint8_t alignment = 4) = 0;

	virtual void Deallocate(void* p) = 0;

	void* GetStart() const
	{
		return start;
	}

	size_t GetSize() const
	{
		return size;
	}

	size_t GetUsedMemory() const
	{
		return used_memory;
	}

	size_t GetNumAllocations() const
	{
		return num_allocations;
	}

protected:
	void*	start;
	size_t	size;
	size_t used_memory;
	size_t num_allocations;
};

namespace allocator
{
	template <class T> T* AllocateNew(Allocator& allocator)
	{
		return new (allocator.Allocate(sizeof(T), alignof(T))) T;
	}

	template <class T> T* AllocateNew(Allocator& allocator, const T& t)
	{
		return new (allocator.Allocate(sizeof(T), alignof(T))) T(t);
	}

	template <class T> void DeallocateDelete(Allocator& allocator, T& t)
	{
		t.~T();
		allocator.Deallocate(&t);
	}

	template <class T> T* AllocateArray(Allocator& allocator, size_t length)
	{
		assert(length != 0);

		uint8_t header_size = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0)
			header_size++;

		T* p = ((T*)allocator.Allocate(sizeof(T)*(length + header_size), alignof(T))) + header_size;

		*(((size_t*)p) - 1) = length;

		for (size_t i = 0; i < length; i++)
			new (&p[i]) T;

		return p;
	}

	template <class T> void DeallocateArray(Allocator& allocator, T* ar)
	{
		assert(ar != null_ptr);

		size_t length = *(((size_t*)ar) - 1);

		for (size_t i = 0; i < length; i++)
			ar[i].~T();

		uint8_t header_size = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0)
			header_size++;

		allocator.Deallocate(ar - header_size);
	}
}

namespace allocator_pointer
{
	inline void* AlignForward(void* address, uint8_t alignment)
	{
		return (void*)((reinterpret_cast<uintptr_t>(address) + static_cast<uintptr_t>(alignment - 1)) & static_cast<uintptr_t>(~(alignment - 1)));
	}

	inline uint8_t AlignForwardAdjustment(const void* address, uint8_t alignment)
	{
		uint8_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(alignment - 1));

		if (adjustment == alignment)
			return 0;

		return adjustment;
	}

	inline uint8_t AlignForwardAdjustmentWithHeader(const void* address, uint8_t alignment, uint8_t header_size)
	{
		uint8_t adjustment = AlignForwardAdjustment(address, alignment);

		uint8_t needed_space = header_size;

		if (adjustment < needed_space)
		{
			needed_space -= adjustment;

			adjustment += alignment * (needed_space / alignment);

			if (needed_space % alignment > 0)
				adjustment += alignment;
		}

		return adjustment;
	}

	inline void* Add(void* p, size_t x)
	{
		return (void*)(reinterpret_cast<uintptr_t>(p) + x);
	}

	inline void* Subtract(void* p, size_t x)
	{
		return (void*)(reinterpret_cast<uintptr_t>(p) - x);
	}
}

#endif // !__ALLOCATOR_H__