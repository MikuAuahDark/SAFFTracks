#include <cstdint>

#include "SADefaultAllocator.hpp"

extern uintptr_t gtasaBase;

void* SADefaultAllocator::operator new(size_t size)
{
	return ((void*(__cdecl *)(size_t)) (gtasaBase + 0x42119a))(size);
}

void SADefaultAllocator::operator delete(void* mem)
{
	return ((void(__cdecl *)(void*)) (gtasaBase + 0x4214bd))(mem);
}
