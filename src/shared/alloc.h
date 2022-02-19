#pragma once

void* AllocAligned(size_t size, size_t alignment);
void FreeAligned(void* alignedMemory);
