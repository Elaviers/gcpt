#pragma once
#include "t_buf.h"
#include "t_list.h"

/*
	Currently relies on assignment / default construction, could easily be beefed up to use copy/move construction properly if need be
*/

template <typename T, typename BlockAlloc = DefaultAllocator<T>, typename BlockListAlloc = DefaultAllocator<T>>
class Stack
{
public:
	using size_type = size_t;

private:
	struct Block
	{
		Buffer<T, BlockAlloc> elements;
		Buffer<T, BlockAlloc>::size_type used;
	};

	using BlockListType = List<Block, BlockListAlloc>;
	using _BlockListAlloc = Utilities::ReplaceParam<BlockListAlloc, Block>;

	class Proxy : public BlockAlloc
	{
	public:
		size_type newBlockSize;
		BlockListType blocks;

		constexpr Proxy(size_type newBlockSize, const BlockAlloc& blockAlloc, const BlockListAlloc& blockListAlloc) noexcept :
			BlockAlloc(blockAlloc), 
			newBlockSize(newBlockSize),
			blocks(blockListAlloc)
		{}

		constexpr Proxy(const Proxy&) = delete;
		constexpr Proxy(Proxy&&) = delete;
	} _proxy;

	size_type& _newBlockSize = _proxy.newBlockSize;
	BlockListType& _blocks = _proxy.blocks;

public:
	constexpr Stack(size_type newBlockSize, const BlockAlloc& blockAlloc = BlockAlloc(), const BlockListAlloc& blockListAlloc = BlockListAlloc()) noexcept :
		_proxy(newBlockSize, blockAlloc, blockListAlloc)
	{}

	constexpr void SetNewBlockSize(size_type newBlockSize) noexcept
	{
		_newBlockSize = newBlockSize;
	}

	constexpr bool IsEmpty() const noexcept { return _blocks.GetSize() == 0; }

	T& Push(const T& value)
	{
		if (auto block = _blocks.Back())
			if (block->used < block->elements.GetSize())
				return block->elements[block->used++] = value;

		Block& newBlock = _blocks.EmplaceBack();
		newBlock.elements.SetSize(_newBlockSize);
		newBlock.used = 1;
		return newBlock.elements[0] = value;
	}

	T* Pop()
	{
		if (auto block = _blocks.Back())
		{
			if (--block->used == 0)
			{
				_blocks.RemoveBack();
				block = _blocks.Back();
			}
			
			return block ? &block->elements[block->used - 1] : nullptr;
		}

		return nullptr;
	}
};
