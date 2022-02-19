#include <cstdlib>
#include <exception>
#include "assert.h"

[[nodiscard]] void* operator new(std::size_t size)
{
	if (size == 0) return nullptr;

	if (void* mem = std::malloc(size))
		return mem;

	throw std::bad_alloc();
}

void operator delete(void* p) noexcept
{
	std::free(p);
}

size_t NextMultipleOf(size_t v, size_t divisor)
{
    Assert(divisor != 0);
    return ((v + divisor - 1) / divisor) * divisor;
}

void* AllocAligned(size_t size, size_t alignment)
{
    if (size == 0)
        return nullptr;

    if (alignment == 0)
        alignment = 1;

    void* memory = std::malloc(size + alignment - 1 + sizeof(void*));
    Assert(memory);

    void* aligned = (void*)NextMultipleOf((size_t)memory + sizeof(void*), alignment);
    *((void**)aligned - 1) = memory;
    return aligned;
}

void FreeAligned(void* alignedMemory)
{
    std::free(*((void**)alignedMemory - 1));
}
