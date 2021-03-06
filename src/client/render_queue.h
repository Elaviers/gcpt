#pragma once
#include "shared/colour.h"
#include "shared/mesh_manager.h"
#include "texture_manager.h"
#include "render_entry.h"
#include "shared/concepts.h"
#include "shared/t_paged_memory.h"
#include "shared/transform.h"

class Projection;

class RenderQueue
{
public:
	template <typename T>
	using AllocatorType = PagedMemory<>::Allocator<T>;
	
	using EntryPtrListType = List<const RenderEntry*, AllocatorType<RenderEntry*>>;

	struct QueueGroup
	{
		const int priority;
		EntryPtrListType queue;

		QueueGroup(int priority, const AllocatorType<RenderEntry*>& alloc)
			: priority(priority), queue(alloc)
		{}
	};

private:
	PagedMemory<> _memory;

	List<RenderEntry, AllocatorType<RenderEntry>> _entries;
	List<QueueGroup> _queues;
	
	EntryPtrListType& _GetQueue(int priority);

public:
	RenderQueue() : _entries(_memory.GetAllocator<RenderEntry>()) {}
	~RenderQueue() {}

	void Clear();
	void Render(ERenderChannels channels, const MeshManager* meshManager, const TextureManager* textureManager, int lightCount) const;

	void AddEntry(const RenderEntry*, int priority = 0);
	RenderEntry& CreateEntry(ERenderChannels renderChannels, int priority = 0);

	RenderEntry& CreateCameraEntry(const Projection& projection, const Transform& transform, ERenderChannels renderChannels = ERenderChannels::ALL, int priority = 0);
};
